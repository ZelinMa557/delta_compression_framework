#include "feature/features.h"
#include "chunk/chunk.h"
#include "utils/gear.h"
#include "utils/rabin.cpp"
#include "utils/simhash.h"
#include <algorithm>
#include <crc32c/crc32c.h>

#include <cstdint>
namespace Delta {
Feature FinesseFeature(std::shared_ptr<Chunk> chunk, const int sf_cnt,
                       const int sf_subf) {
  int sub_chunk_length = chunk->len() / (sf_subf * sf_cnt);
  uint8_t *content = chunk->buf();
  std::vector<uint64_t> sub_features(sf_cnt * sf_subf, 0);
  std::vector<uint64_t> super_features(sf_cnt, 0);

  // calculate sub features.
  for (int i = 0; i < sub_features.size(); i++) {
    rabin_t rabin_ctx;
    rabin_init(&rabin_ctx);
    for (int j = 0; j < sub_chunk_length; j++) {
      rabin_append(&rabin_ctx, content[j]);
      sub_features[i] = std::max(rabin_ctx.digest, sub_features[i]);
    }
    content += sub_chunk_length;
  }

  // group the sub features into super features.
  for (int i = 0; i < sub_features.size(); i += sf_subf) {
    std::sort(sub_features.begin() + i, sub_features.begin() + i + sf_subf);
  }
  for (int i = 0; i < sf_cnt; i++) {
    rabin_t rabin_ctx;
    rabin_init(&rabin_ctx);
    for (int j = 0; j < sf_subf; j++) {
      auto sub_feature = sub_features[sf_subf * i + j];
      auto data_ptr = (uint8_t *)&sub_feature;
      for (int k = 0; k < 8; k++) {
        rabin_append(&rabin_ctx, data_ptr[k]);
      }
    }
    super_features[i] = rabin_ctx.digest;
  }
  return super_features;
}

static uint64_t M[] = {
    1, 4, 9, 23, 37, 45, 179, 286, 437, 920, 1014, 2987,
};

static uint64_t A[] = {
    982, 1343, 732, 334, 439, 832, 892, 751, 21, 90, 55, 16,
};

Feature OdessFeature(std::shared_ptr<Chunk> chunk, const int sf_cnt,
                     const int sf_subf, uint64_t mask) {
  int features_num = sf_cnt * sf_subf;
  std::vector<uint64_t> sub_features(features_num, 0);
  std::vector<uint64_t> super_features(sf_cnt, 0);

  int chunk_length = chunk->len();
  uint8_t *content = chunk->buf();
  uint64_t finger_print = 0;
  // calculate sub features.
  for (int i = 0; i < chunk_length; i++) {
    finger_print = (finger_print << 1) + GEAR_TABLE[content[i]];
    if ((finger_print & mask) == 0) {
      for (int j = 0; j < features_num; j++) {
        uint64_t transform = (M[j] * finger_print + A[j]) % (1LL << 32);
        // we need to guarantee that when sub_features[i] is not inited,
        // always set its value
        if (sub_features[j] >= transform || 0 == sub_features[j])
          sub_features[j] = transform;
      }
    }
  }

  // group sub features into super features.
  auto hash_buf = (const uint8_t *const)(sub_features.data());
  for (int i = 0; i < sf_cnt; i++) {
    uint64_t hash_value = 0;
    auto this_hash_buf = hash_buf + i * sf_subf * sizeof(uint64_t);
    for (int j = 0; j < sf_subf * 8; j++) {
      hash_value = (hash_value << 1) + GEAR_TABLE[this_hash_buf[j]];
    }
    super_features[i] = hash_value;
  }
  return super_features;
}

Feature CRCSimHashFeature(std::shared_ptr<Chunk> chunk, const int sub_chunk) {
  std::vector<uint32_t> crc_result(sub_chunk, 0);
  int sub_chunk_size = chunk->len() / sub_chunk;
  auto buf = chunk->buf();
  for (int i = 0; i < sub_chunk; i++) {
    crc_result[i] = crc32c::Crc32c(buf, sub_chunk_size);
    buf += sub_chunk_size;
  }
  return simhash::simhash_ex(crc_result);
}

Feature SimHashFeature(std::shared_ptr<Chunk> chunk) {
  return simhash::simhash_ex(chunk->buf(), chunk->len());
}
} // namespace Delta