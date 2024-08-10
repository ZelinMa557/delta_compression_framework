#include "index/naive_multi_index_hamming.h"
#include "chunk/chunk.h"
#include "utils/hamming_distance.h"
#include <assert.h>
namespace Delta {
std::optional<chunk_id> NaiveMIH::GetBaseChunkID(const Feature &feat) {
  uint64_t signature = std::get<uint64_t>(feat);
  std::optional<chunk_id> best_chunk_id = std::nullopt;
  int min_distance = 4;
  int best_founds = 0;
  if (hashes_.empty()) {
    for (int i = 0; i < 8; i++)
      hashes_.push_back({});
  }
  for (int i = 0; i < 8; i++) {
    uint8_t sub_sig = ((signature >> (i * 8)) & 0xff);
    if (!hashes_[i].count(sub_sig))
      continue;
    for (const auto &sig : hashes_[i][sub_sig]) {
      auto distance = hammingDistance(signature, sig);
      if (distance < min_distance) {
        min_distance = distance;
        assert(sig_chunk_id_.count(sig));
        best_chunk_id = sig_chunk_id_[sig];
        best_founds = 1;
      } else if (distance == min_distance && sig_chunk_id_[sig] != best_chunk_id) {
        best_founds++;
      }
    }
  }
  // if (best_founds > 1)
  // printf("best dis %d found %d total base chunk %d\n", min_distance, best_founds, this->sig_chunk_id_.size());
  return best_chunk_id;
}

void NaiveMIH::AddFeature(const Feature &feat, chunk_id id) {
  uint64_t signature = std::get<uint64_t>(feat);
  for (int i = 0; i < 8; i++) {
    uint8_t sub_sig = ((signature >> (i * 8)) & 0xff);
    hashes_[i][sub_sig].push_back(signature);
  }
  sig_chunk_id_[signature] = id;
}
} // namespace Delta