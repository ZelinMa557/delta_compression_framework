#include "chunk/chunk.h"
#include "index/super_feature_index.h"
#include "feature/features.h"
#include <assert.h>
#include <fstream>
#include <glog/logging.h>
namespace Delta {
using chunk_id = uint32_t;
std::optional<chunk_id>
SuperFeatureIndex::GetBaseChunkID(std::shared_ptr<Chunk> chunk,
                                  bool add_new_base_chunk) {
  auto super_feature = get_feature_(chunk, super_feature_count_);
  std::optional<chunk_id> result = std::nullopt;
  for (int i = 0; i < super_feature_count_; i++) {
    // get a matched super feature
    if (index_[i].count(super_feature[i])) {
      // first fit
      result = index_[i][super_feature[i]];
      break;
    }
  }
  if (add_new_base_chunk && !result.has_value()) {
    for (int i = 0; i < super_feature_count_; i++) {
      index_[i][super_feature[i]] = chunk->id();
    }
    result = chunk->id();
  }
  return result;
}

bool SuperFeatureIndex::DumpToFile(const std::string &path) {
  std::ofstream outFile(path, std::ios::binary);
  if (!outFile) {
    LOG(FATAL) << "SuperFeatureIndex::DumpToFile: cannot open output file "
              << path;
    return false;
  }
  auto write_uint64 = [&](uint64_t data) {
    outFile.write(reinterpret_cast<const char *>(&data), sizeof(uint64_t));
  };
  auto write_uint32 = [&](uint32_t data) {
    outFile.write(reinterpret_cast<const char *>(&data), sizeof(uint32_t));
  };
  write_uint32(super_feature_count_);
  for (int i = 0; i < super_feature_count_; i++) {
    write_uint64(index_[i].size());
    for (const auto &[k, v] : index_[i]) {
      write_uint64(k);
      write_uint32(v);
    }
  }
  return true;
}

bool SuperFeatureIndex::RecoverFromFile(const std::string &path) {
  std::ifstream inFile(path, std::ios::binary);
  if (!inFile) {
    LOG(FATAL) << "SuperFeatureIndex::RecoverFromFile: cannot open output file "
              << path;
    return false;
  }
  int super_feature_count = 0;
  inFile.read(reinterpret_cast<char *>(&super_feature_count), sizeof(int));
  if (super_feature_count_ != super_feature_count) {
    LOG(FATAL) << "super feature count changed after recover, abort";
    return false;
  }
  auto read_uint64 = [&]() -> uint64_t {
    uint64_t result = 0;
    inFile.read(reinterpret_cast<char *>(&result), sizeof(uint64_t));
    return result;
  };
  auto read_uint32 = [&]() -> uint32_t {
    uint32_t result = 0;
    inFile.read(reinterpret_cast<char *>(&result), sizeof(uint32_t));
    return result;
  };
  for (int i = 0; i < super_feature_count_; i++) {
    uint64_t mapSize = read_uint64();
    for (int j = 0; j < mapSize; j++) {
      auto feature = read_uint64();
      auto chunk_id = read_uint32();
      index_[i][feature] = chunk_id;
    }
  }
  return true;
}
} // namespace Delta