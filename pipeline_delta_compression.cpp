#include "pipeline_delta_compression.h"
#include <glog/logging.h>
#define write_to_queue(TASK_NAME, CTX_NAME)                                    \
  do {                                                                         \
    std::unique_lock<std::mutex> lock(TASK_NAME##_lock_);                      \
    TASK_NAME##_cv_.wait(                                                      \
        lock, [this] { return TASK_NAME##_queue_.size() < max_queue_len_; });  \
    TASK_NAME##_queue_.push(CTX_NAME);                                         \
    TASK_NAME##_cv_.notify_all();                                              \
  } while (0)

#define get_context_from_queue(TASK_NAME, CTX_NAME)                            \
  do {                                                                         \
    std::unique_lock<std::mutex> lock(TASK_NAME##_lock_);                      \
    TASK_NAME##_cv_.wait(lock, [this] {                                        \
      return !TASK_NAME##_queue_.empty() || TASK_NAME##_done_;                 \
    });                                                                        \
    if (!TASK_NAME##_queue_.empty()) {                                         \
      CTX_NAME = TASK_NAME##_queue_.front();                                   \
      TASK_NAME##_queue_.pop();                                                \
    }                                                                          \
    TASK_NAME##_cv_.notify_all();                                              \
  } while (0);

#define task_done(TASK_NAME)                                                   \
  do {                                                                         \
    TASK_NAME##_done_ = true;                                                  \
    TASK_NAME##_cv_.notify_all();                                              \
  } while (0)

namespace Delta {
void PipelineDeltaCompression::ChunkingWorker() {
  while (true) {
    auto chunk = this->chunker_->GetNextChunk();
    if (!chunk)
      break;
    context *context_ = new context();
    context_->chunk = chunk;
    context_->chunk_type = 0;
    write_to_queue(chunk, context_);
  }
  task_done(chunk);
}

void PipelineDeltaCompression::DedupWorker() {
  while (true) {
    context *context_ = nullptr;
    get_context_from_queue(chunk, context_);
    if (nullptr == context_)
      break;
    uint32_t dedup_base_id = dedup_->ProcessChunk(context_->chunk);
    if (dedup_base_id != context_->chunk->id()) {
      context_->chunk_type = DuplicateChunk;
      context_->base_chunk_id = dedup_base_id;
    }
    write_to_queue(dedup, context_);
  }
  task_done(dedup);
}

void PipelineDeltaCompression::FeatureCalcWorker() {
  while (true) {
    context *context_ = nullptr;
    get_context_from_queue(dedup, context_);
    if (nullptr == context_)
      break;
    if (context_->chunk_type != DuplicateChunk) {
      context_->feature = (*feature_)(context_->chunk);
    }
    write_to_queue(feature, context_);
  }
  task_done(feature);
}

void PipelineDeltaCompression::ChunkMatchWorker() {
  while (true) {
    context *context_ = nullptr;
    get_context_from_queue(feature, context_);
    if (nullptr == context_)
      break;
    if (context_->chunk_type != DuplicateChunk) {
      const auto &feature = context_->feature;
      auto base_chunk_id = index_->GetBaseChunkID(feature);
      if (!base_chunk_id.has_value()) {
        context_->chunk_type = BaseChunk;
        index_->AddFeature(feature, context_->chunk->id());
      } else {
        context_->chunk_type = DeltaChunk;
        context_->base_chunk_id = base_chunk_id.value();
      }
    }
    write_to_queue(match, context_);
  }
  task_done(match);
}

void PipelineDeltaCompression::ChunkWriteWorker() {
  while (true) {
    context *context_ = nullptr;
    get_context_from_queue(match, context_);
    if (nullptr == context_)
      break;
    auto chunk_len = context_->chunk->len();
    total_size_origin_ += chunk_len;
    switch (context_->chunk_type) {
    case DuplicateChunk:
      duplicate_chunk_count_++;
      storage_->WriteDuplicateChunk(context_->chunk, context_->base_chunk_id);
      break;
    case BaseChunk:
      base_chunk_count_++;
      total_size_compressed_ += chunk_len;
      storage_->WriteBaseChunk(context_->chunk);
      break;
    case DeltaChunk:
      delta_chunk_count_++;
      chunk_size_before_delta_ += chunk_len;
      {
        auto delta_chunk = storage_->GetDeltaEncodedChunk(
            context_->chunk, context_->base_chunk_id);
        chunk_size_after_delta_ += delta_chunk->len();
        total_size_compressed_ += delta_chunk->len();
        storage_->WriteDeltaChunk(delta_chunk, context_->base_chunk_id);
      }
      break;
    default:
      LOG(FATAL)
          << "PipelineDeltaCompression::ChunkWriteWorker(): unknown chunk type "
          << context_->chunk_type;
    }
    processed_chunk_++;
    delete context_;
  }
}

void PipelineDeltaCompression::AddFile(const std::string &file_name) {
  FileMeta file_meta;
  file_meta.file_name = file_name;
  file_meta.start_chunk_id = processed_chunk_;

  chunker_->ReinitWithFile(file_name);

  chunk_done_ = false;
  dedup_done_ = false;
  feature_done_ = false;
  match_done_ = false;

  std::thread chunk_worker(&PipelineDeltaCompression::ChunkingWorker, this);
  std::thread dedup_worker(&PipelineDeltaCompression::DedupWorker, this);
  std::thread feature_worker(&PipelineDeltaCompression::FeatureCalcWorker,
                             this);
  std::thread match_worker(&PipelineDeltaCompression::ChunkMatchWorker, this);
  std::thread write_worker(&PipelineDeltaCompression::ChunkWriteWorker, this);

  chunk_worker.join();
  dedup_worker.join();
  feature_worker.join();
  match_worker.join();
  write_worker.join();
  file_meta.end_chunk_id = (processed_chunk_ - 1);
  file_meta_writer_.Write(file_meta);
}
} // namespace Delta