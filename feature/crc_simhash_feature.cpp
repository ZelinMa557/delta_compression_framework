#include "chunk/chunk.h"
#include "feature/features.h"
#include "utils/simhash.h"
#include <crc32c/crc32c.h>

namespace Delta {
uint64_t CRCSimHashFeature(std::shared_ptr<Chunk> chunk,
                           const int sub_chunk) {
  // std::vector<uint32_t> crc_result(sub_chunk, 0);
  // int sub_chunk_size = chunk->len() / sub_chunk;
  // auto buf = chunk->buf();
  // for (int i = 0; i < sub_chunk; i++) {
  //   crc_result[i] = crc32c::Crc32c(buf, sub_chunk_size);
  //   buf += sub_chunk_size;
  // }
  std::vector<uint32_t> crc_result;
  const uint32_t mask = 63, minl = 128/4, maxl = 512/4;
  uint8_t *buf = chunk->buf();
  uint32_t crc = 0;
  int curr_size = 0;
  for (int i = 0; i < chunk->len(); i++) {
    crc = crc32c::Extend(crc, buf+i, 1);
    curr_size++;
    if (curr_size >= minl && (crc & mask) == 0 || curr_size >= maxl) {
      crc_result.push_back(crc);
      crc = 0;
      curr_size = 0;
    }
  }
  if (curr_size != 0)
    crc_result.push_back(crc);
  return simhash::simhash_ex(crc_result);
}
} // namespace Delta