#include "chunk/chunk.h"
#include "feature/features.h"
#include "utils/gear.h"
#include <cstdint>
#include <glog/logging.h>
namespace Delta {
static uint64_t M[] = {
    1, 4, 9, 23, 37, 45, 179, 286, 437, 920, 1014, 2987,
};

static uint64_t A[] = {
    982, 1343, 732, 334, 439, 832, 892, 751, 21, 90, 55, 16,
};

std::vector<uint64_t> OdessFeature(std::shared_ptr<Chunk> chunk,
                                   const int sf_cnt, const int sf_subf,
                                   uint64_t mask) {
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
} // namespace Delta