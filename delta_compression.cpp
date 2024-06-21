#include "chunk/chunk.h"
#include "chunk/fast_cdc.h"
#include "chunk/rabin_cdc.h"
#include "encoder/xdelta.h"
#include "index/super_feature_index.h"
#include "storage/storage.h"
#include "delta_compression.h"
#include <string>
#include <vector>
namespace Delta {
void DeltaCompression::AddFile(const std::string &file_name) {
  this->chunker_->ReinitWithFile(file_name);
  while (true) {
    auto chunk = chunker_->GetNextChunk();
    if (!chunk)
      break;
    auto base_chunk_id = index_->GetBaseChunkID(chunk, true);
    if (base_chunk_id.value() == chunk->id()) {
      storage_.WriteBaseChunk(chunk);
    } else {
      storage_.WriteDeltaChunk(chunk, base_chunk_id.value());
    }
  }
}

std::unique_ptr<DeltaCompression>
DeltaCompression::MakeFinesse(std::string out_data_path,
                              std::string out_meta_path,
                              std::string index_path) {
  return std::make_unique<DeltaCompression>(
      out_data_path, out_meta_path, index_path, std::make_unique<RabinCDC>(),
      std::make_unique<XDelta>(),
      std::make_unique<SuperFeatureIndex>(
          SuperFeatureIndex::SuperFeatureType::Finesse));
}

std::unique_ptr<DeltaCompression>
DeltaCompression::MakeOdess(std::string out_data_path,
                            std::string out_meta_path, std::string index_path) {
  return std::make_unique<DeltaCompression>(
      out_data_path, out_meta_path, index_path, std::make_unique<FastCDC>(),
      std::make_unique<XDelta>(),
      std::make_unique<SuperFeatureIndex>(
          SuperFeatureIndex::SuperFeatureType::Odess));
}
} // namespace Delta