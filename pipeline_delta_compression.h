#pragma once
#include "delta_compression.h"
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>
namespace Delta {
class PipelineDeltaCompression : public DeltaCompression {
public:
  PipelineDeltaCompression() : DeltaCompression() {}
  void AddFile(const std::string &file_name) override;

private:
  const size_t max_queue_len_ = 128;
  void ChunkingWorker();
  void DedupWorker();
  void FeatureCalcWorker();
  void ChunkMatchWorker();
  void ChunkWriteWorker();

  size_t processed_chunk_ = 0;
  struct context {
    std::shared_ptr<Chunk> chunk;
    uint32_t base_chunk_id;
    uint8_t chunk_type;
    Feature feature;
  };
  std::queue<context*> chunk_queue_;
  std::queue<context*> dedup_queue_;
  std::queue<context*> feature_queue_;
  std::queue<context*> match_queue_;
  

  std::condition_variable chunk_cv_;
  std::condition_variable dedup_cv_;
  std::condition_variable feature_cv_;
  std::condition_variable match_cv_;
  
  std::mutex chunk_lock_;
  std::mutex dedup_lock_;
  std::mutex feature_lock_;
  std::mutex match_lock_;


  alignas(64) bool chunk_done_;
  alignas(64) bool dedup_done_;
  alignas(64) bool feature_done_;
  alignas(64) bool match_done_;

};
} // namespace Delta