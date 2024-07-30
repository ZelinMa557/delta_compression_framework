#pragma once
#include "index/index.h"
#include "feature/features.h"
#include <vector>
#include <unordered_map>
namespace Delta {
// this is only for experiments
class NaiveMIH : public Index {
public:
  std::optional<chunk_id> GetBaseChunkID(const Feature &feat);
  void AddFeature(const Feature &feat, chunk_id id);
  bool RecoverFromFile(const std::string &path) { return true; }
  bool DumpToFile(const std::string &path) { return true;}
private:
  std::vector<std::unordered_map<uint8_t, std::vector<uint64_t>>> hashes_;
  std::unordered_map<uint64_t, uint32_t> sig_chunk_id_;
};
} // namespace Delta