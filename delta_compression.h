#pragma once
#include "chunk/chunker.h"
#include "encoder/encoder.h"
#include "index/index.h"
#include "storage/file_meta.h"
#include "storage/storage.h"
#include "dedup/dedup.h"
#include <memory>
#include <string>
namespace Delta {
class DeltaCompression {
public:
  DeltaCompression();
  ~DeltaCompression();
  virtual void AddFile(const std::string &file_name);

protected:
  std::string out_data_path_;
  std::string out_meta_path_;
  std::string index_path_;

  std::unique_ptr<Chunker> chunker_;
  std::unique_ptr<Index> index_;
  std::unique_ptr<Dedup> dedup_;
  std::unique_ptr<Storage> storage_;
  std::unique_ptr<FeatureCalculator> feature_;

  FileMetaWriter file_meta_writer_;

  uint32_t base_chunk_count_ = 0;
  uint32_t delta_chunk_count_ = 0;
  uint32_t duplicate_chunk_count_ = 0;

  size_t total_size_origin_ = 0;
  size_t total_size_compressed_ = 0;
  size_t chunk_size_before_delta_ = 0;
  size_t chunk_size_after_delta_ = 0;
};
} // namespace Delta