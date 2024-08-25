#include "index/best_fit_index.h"
namespace Delta {
std::optional<chunk_id> BestFitIndex::GetBaseChunkID(const Feature &feat) {
  const auto &features = std::get<std::vector<uint64_t>>(feat);
  std::unordered_map<chunk_id, uint32_t> match_count;
  for (int i = 0; i < feature_count_; i++) {
    const auto &index_i = index_[i];
    const uint64_t feature = features[i];
    if (!index_i.count(feature))
      continue;
    const auto &matched_chunk_ids = index_i.at(feature);
    for (const auto &id: matched_chunk_ids) {
      match_count[id]++;
    }
  }
  if (match_count.empty())
    return std::nullopt;
  uint32_t max_match = 0, max_match_id = -1;
  for (const auto &[chunk_id, count]: match_count) {
    if (count > max_match) {
      max_match_id = chunk_id;
      max_match = count;
    }
  }
  if (max_match < 4)
    return std::nullopt;
  return max_match_id;
}

void BestFitIndex::AddFeature(const Feature &feat, chunk_id id) {
  const auto &features = std::get<std::vector<uint64_t>>(feat);
  for (int i = 0; i < feature_count_; i++) {
    index_[i][features[i]].push_back(id);
  }
}
} // namespace Delta