#include "chunk/chunk.h"
#include "chunk/crc_cdc.h"
#include "feature/features.h"
#include "utils/simhash.h"
#include <crc32c/crc32c.h>

namespace Delta {
namespace {
constexpr int sub_chunk_count = 32;
constexpr int stop_mask = 63;
constexpr int min_sub_chunk_size = 128;
constexpr int max_sub_chunk_size = 512;

}
uint64_t CRCSimHashFeature(std::shared_ptr<Chunk> chunk,
                           const int sub_chunk) {
  // std::vector<uint32_t> crc_result(sub_chunk, 0);
  // int sub_chunk_size = chunk->len() / sub_chunk;
  // auto buf = chunk->buf();
  // for (int i = 0; i < sub_chunk; i++) {
  //   crc_result[i] = crc32c::Crc32c(buf, sub_chunk_size);
  //   buf += sub_chunk_size;
  // }
  // return simhash::simhash_ex(crc_result);
  auto buf = chunk->buf();
  int remaining_length = chunk->len();
  std::vector<uint32_t> crc_sigs;
  crc_sigs.reserve(sub_chunk_count);
  while (remaining_length > 0) {
    if (remaining_length < max_sub_chunk_size) {
      crc_sigs.push_back(crc32c::Extend(0, buf, remaining_length));
      break;
    }
    uint32_t crc_sig = crc32c::Extend(0, buf, min_sub_chunk_size);
    int cur_chk_size = min_sub_chunk_size;
    for (; cur_chk_size < max_sub_chunk_size; cur_chk_size += 4) {
      if ((crc_sig & stop_mask) == 0)
        break;
      crc_sig = crc32c::Extend(crc_sig, buf + cur_chk_size, 4);
    }
    crc_sigs.push_back(crc_sig);
    remaining_length -= cur_chk_size;
    buf += cur_chk_size;
  }
  return simhash::simhash_ex(crc_sigs);
}
} // namespace Delta