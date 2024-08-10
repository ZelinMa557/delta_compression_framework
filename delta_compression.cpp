#include "delta_compression.h"
#include "chunk/chunk.h"
#include "chunk/crc_cdc.h"
#include "chunk/fast_cdc.h"
#include "chunk/rabin_cdc.h"
#include "config.h"
#include "encoder/xdelta.h"
#include "index/naive_multi_index_hamming.h"
#include "index/super_feature_index.h"
#include "storage/storage.h"
#include <glog/logging.h>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
namespace Delta {
void DeltaCompression::AddFile(const std::string &file_name) {
  FileMeta file_meta;
  file_meta.file_name = file_name;
  file_meta.start_chunk_id = -1;
  this->chunker_->ReinitWithFile(file_name);
  while (true) {
    auto chunk = chunker_->GetNextChunk();
    if (nullptr == chunk)
      break;
    if (-1 == file_meta.start_chunk_id)
      file_meta.start_chunk_id = chunk->id();
    uint32_t dedup_base_id = dedup_->ProcessChunk(chunk);
    total_size_origin_ += chunk->len();
    // duplicate chunk
    if (dedup_base_id != chunk->id()) {
      storage_->WriteDuplicateChunk(chunk, dedup_base_id);
      duplicate_chunk_count_++;
      continue;
    }

    auto write_base_chunk = [this](const std::shared_ptr<Chunk> &chunk) {
      storage_->WriteBaseChunk(chunk);
      base_chunk_count_++;
      total_size_compressed_ += chunk->len();
    };

    auto write_delta_chunk = [this](const std::shared_ptr<Chunk> &chunk,
                                    const std::shared_ptr<Chunk> &delta_chunk,
                                    const uint32_t base_chunk_id) {
      chunk_size_before_delta_ += chunk->len();
      storage_->WriteDeltaChunk(delta_chunk, base_chunk_id);
      delta_chunk_count_++;
      chunk_size_after_delta_ += delta_chunk->len();
      total_size_compressed_ += delta_chunk->len();
    };

    auto feature = (*feature_)(chunk);
    auto base_chunk_id = index_->GetBaseChunkID(feature);
    if (!base_chunk_id.has_value()) {
      index_->AddFeature(feature, chunk->id());
      write_base_chunk(chunk);
      continue;
    }

    auto delta_chunk = storage_->GetDeltaEncodedChunk(chunk, base_chunk_id.value());
    // if ((*filter_)(chunk, delta_chunk)) {
    //   index_->AddFeature(feature, chunk->id());
    //   write_base_chunk(chunk);
    //   continue;
    // }
    write_delta_chunk(chunk, delta_chunk, base_chunk_id.value());
    file_meta.end_chunk_id = chunk->id();
  }
  file_meta_writer_.Write(file_meta);
}

DeltaCompression::~DeltaCompression() {
  auto print_ratio = [](uint32_t a, uint32_t b) {
    double ratio = (double)a / (double)b;
    std::cout << std::fixed << std::setprecision(1);
    std::cout << "(" << ratio * 100 << "%)" << std::endl;
    std::cout << std::defaultfloat;
  };
  uint32_t chunk_count =
      base_chunk_count_ + delta_chunk_count_ + duplicate_chunk_count_;
  std::cout << "Total chunk count: " << chunk_count << std::endl;
  std::cout << "Base chunk count: " << base_chunk_count_;
  print_ratio(base_chunk_count_, chunk_count);
  std::cout << "Delta chunk count: " << delta_chunk_count_;
  print_ratio(delta_chunk_count_, chunk_count);
  std::cout << "Duplicate chunk count: " << duplicate_chunk_count_;
  print_ratio(duplicate_chunk_count_, chunk_count);
  std::cout << "DCR (Delta Compression Ratio): ";
  print_ratio(total_size_origin_, total_size_compressed_);
  std::cout << "DCE (Delta Compression Efficiency): ";
  print_ratio(chunk_size_after_delta_, chunk_size_before_delta_);
}

DeltaCompression::DeltaCompression() {
  auto config = Config::Instance().get();
  auto index_path = *config->get_as<std::string>("index_path");
  auto chunk_data_path = *config->get_as<std::string>("chunk_data_path");
  auto chunk_meta_path = *config->get_as<std::string>("chunk_meta_path");
  auto file_meta_path = *config->get_as<std::string>("file_meta_path");
  auto dedup_index_path = *config->get_as<std::string>("dedup_index_path");

  auto chunker = config->get_table("chunker");
  auto chunker_type = *chunker->get_as<std::string>("type");
  if (chunker_type == "rabin-cdc" || chunker_type == "fast-cdc" ||
      chunker_type == "crc-cdc") {
    auto min_chunk_size = *chunker->get_as<int64_t>("min_chunk_size");
    auto max_chunk_size = *chunker->get_as<int64_t>("max_chunk_size");
    auto stop_mask = *chunker->get_as<int64_t>("stop_mask");
    if (chunker_type == "rabin-cdc") {
      this->chunker_ =
          std::make_unique<RabinCDC>(min_chunk_size, max_chunk_size, stop_mask);
      LOG(INFO) << "Add RabinCDC chunker, min_chunk_size=" << min_chunk_size
                << " max_chunk_size=" << max_chunk_size
                << " stop_mask=" << stop_mask;
    } else if (chunker_type == "fast-cdc") {
      this->chunker_ =
          std::make_unique<FastCDC>(min_chunk_size, max_chunk_size, stop_mask);
      LOG(INFO) << "Add FastCDC chunker, min_chunk_size=" << min_chunk_size
                << " max_chunk_size=" << max_chunk_size
                << " stop_mask=" << stop_mask;
    } else if (chunker_type == "crc-cdc") {
      this->chunker_ =
          std::make_unique<CRC_CDC>(min_chunk_size, max_chunk_size, stop_mask);
      LOG(INFO) << "Add CRC_CDC chunker, min_chunk_size=" << min_chunk_size
                << " max_chunk_size=" << max_chunk_size
                << " stop_mask=" << stop_mask;
    }
  } else {
    LOG(FATAL) << "Unknown chunker type " << chunker_type;
  }

  auto feature = config->get_table("feature");
  auto feature_type = *feature->get_as<std::string>("type");

  if (feature_type == "finesse") {
    this->feature_ = std::make_unique<FinesseFeature>(default_finesse_sf_cnt, default_finesse_sf_subf);
    this->index_ = std::make_unique<SuperFeatureIndex>();
  } else if (feature_type == "odess") {
    this->feature_ = std::make_unique<OdessFeature>(default_odess_sf_cnt, default_odess_sf_subf, default_odess_mask);
    this->index_ = std::make_unique<SuperFeatureIndex>();
  } else if (feature_type == "crc-simhash") {
    this->feature_ = std::make_unique<SimHashFeature>();
    this->index_ = std::make_unique<NaiveMIH>();
  } else {
    LOG(FATAL) << "Unknown feature type " << feature_type;
  }

  this->dedup_ = std::make_unique<Dedup>(dedup_index_path);

  auto storage = config->get_table("storage");
  auto encoder_name = *storage->get_as<std::string>("encoder");
  auto cache_size = *storage->get_as<int64_t>("cache_size");
  std::unique_ptr<Encoder> encoder;
  if (encoder_name == "xdelta") {
    encoder = std::make_unique<XDelta>();
  } else {
    LOG(FATAL) << "Unknown encoder type " << encoder_name;
  }
  this->storage_ = std::make_unique<Storage>(
      chunk_data_path, chunk_meta_path, std::move(encoder), true, cache_size);
  this->file_meta_writer_.Init(file_meta_path);

  // TODO
  this->filter_ = std::make_unique<NotFilterStrategy>();
}
} // namespace Delta