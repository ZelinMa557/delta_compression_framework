#include "index/naive_multi_index_hamming.h"
#include "chunk/chunk.h"
#include "utils/hamming_distance.h"
#include <assert.h>
namespace Delta {
std::optional<chunk_id> NaiveMIH::GetBaseChunkID(const Feature &feat) {
  uint64_t signature = std::get<uint64_t>(feat);
  std::optional<chunk_id> best_chunk_id = std::nullopt;
  int min_distance = 4;
  if (hashes_.empty()) {
    for (int i = 0; i < 4; i++)
      hashes_.push_back({});
  }
  for (int i = 0; i < 4; i++) {
    uint8_t sub_sig = ((signature >> (i * 4)) & 0xffff);
    if (!hashes_[i].count(sub_sig))
      continue;
    for (const auto &sig : hashes_[i][sub_sig]) {
      auto distance = hammingDistance(signature, sig);
      if (distance < min_distance) {
        min_distance = distance;
        assert(sig_chunk_id_.count(sig));
        best_chunk_id = sig_chunk_id_[sig];
      }
    }
  }
  return best_chunk_id;
}

void NaiveMIH::AddFeature(const Feature &feat, chunk_id id) {
  uint64_t signature = std::get<uint64_t>(feat);
  for (int i = 0; i < 4; i++) {
    uint8_t sub_sig = ((signature >> (i * 8)) & 0xffff);
    hashes_[i][sub_sig].push_back(signature);
  }
  sig_chunk_id_[signature] = id;
}
} // namespace Delta