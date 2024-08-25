#include "index/palantir_index.h"
namespace Delta {
std::optional<chunk_id> PalantirIndex::GetBaseChunkID(const Feature &feat) {
  const auto &features = std::get<std::vector<std::vector<uint64_t>>>(feat);
  for (int i = 0; i < features.size(); i++) {
    auto chunk = levels_[i]->GetBaseChunkID(features[i]);
    if (chunk.has_value())
      return chunk;
  }
  return std::nullopt;
}

void PalantirIndex::AddFeature(const Feature &feat, chunk_id id) {
  const auto &features = std::get<std::vector<std::vector<uint64_t>>>(feat);
  for (int i = 0; i < features.size(); i++) {
    levels_[i]->AddFeature(features[i], id);
  }
}
} // namespace Delta