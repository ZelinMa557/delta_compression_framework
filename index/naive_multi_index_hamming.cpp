#include "index/naive_multi_index_hamming.h"
#include "utils/hamming_distance.h"
#include "chunk/chunk.h"
#include <assert.h>
namespace Delta {
std::optional<chunk_id> NaiveMIH::GetBaseChunkID(std::shared_ptr<Chunk> chunk, bool add_new_base_chunk) {
  uint64_t signature = CRCSimHashFeature(chunk);
  chunk_id best_chunk_id = -1;
  int min_distance = 2;
  if (hashes_.empty()) {
    for (int i = 0; i < 8; i++)
      hashes_.push_back({});
  }
  for (int i = 0; i < 8; i++) {
    uint8_t sub_sig = ((signature >> (i * 8)) & 0xff);
    if (!hashes_[i].count(sub_sig))
      continue;
    for (const auto &sig: hashes_[i][sub_sig]) {
      auto distance = hammingDistance(signature, sig);
      if (distance < min_distance) {
        min_distance = distance;
        assert(sig_chunk_id_.count(sig));
        best_chunk_id = sig_chunk_id_[sig];
      }
    }
  }
  if (-1 == best_chunk_id) {
    best_chunk_id = chunk->id();
    for (int i = 0; i < 8; i++) {
    uint8_t sub_sig = ((signature >> (i * 8)) & 0xff);
    hashes_[i][sub_sig].push_back(signature);
    }
    sig_chunk_id_[signature] = chunk->id();
  }
  return best_chunk_id;
}
} // namespace Delta