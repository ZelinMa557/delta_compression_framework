#include "chunk/chunk.h"
#include "feature/features.h"
#include "utils/simhash.h"
#include <crc32c/crc32c.h>

namespace Delta {
uint64_t CRCSimHashFeature(std::shared_ptr<Chunk> chunk,
                           const int sub_chunk) {
  std::vector<uint32_t> crc_result(sub_chunk, 0);
  int sub_chunk_size = chunk->len() / sub_chunk;
  auto buf = chunk->buf();
  for (int i = 0; i < sub_chunk; i++) {
    crc_result[i] = crc32c::Crc32c(buf, sub_chunk_size);
    buf += sub_chunk_size;
  }
  return simhash::simhash_ex(crc_result);
}
} // namespace Delta