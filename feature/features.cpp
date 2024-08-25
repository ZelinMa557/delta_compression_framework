#include "feature/features.h"
#include "chunk/chunk.h"
#include "utils/gear.h"
#include "utils/rabin.cpp"
#include "utils/simhash.h"
#include <algorithm>
#include <queue>
#include <crc32c/crc32c.h>
#include <iostream>
#include <iomanip>
#include <cstdint>
namespace Delta {
Feature FinesseFeature::operator()(std::shared_ptr<Chunk> chunk) {
  int sub_chunk_length = chunk->len() / (sf_subf_ * sf_cnt_);
  uint8_t *content = chunk->buf();
  std::vector<uint64_t> sub_features(sf_cnt_ * sf_subf_, 0);
  std::vector<uint64_t> super_features(sf_cnt_, 0);

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
  for (int i = 0; i < sub_features.size(); i += sf_subf_) {
    std::sort(sub_features.begin() + i, sub_features.begin() + i + sf_subf_);
  }
  for (int i = 0; i < sf_cnt_; i++) {
    rabin_t rabin_ctx;
    rabin_init(&rabin_ctx);
    for (int j = 0; j < sf_subf_; j++) {
      auto sub_feature = sub_features[sf_subf_ * i + j];
      auto data_ptr = (uint8_t *)&sub_feature;
      for (int k = 0; k < 8; k++) {
        rabin_append(&rabin_ctx, data_ptr[k]);
      }
    }
    super_features[i] = rabin_ctx.digest;
  }
  return super_features;
}

static uint32_t M[] = {
    0x5b49898a, 0xe4f94e27, 0x95f658b2, 0x8f9c99fc, 0xeba8d4d8, 0xba2c8e92,
    0xa868aeb4, 0xd767df82, 0x843606a4, 0xc1e70129, 0x32d9d1b0, 0xeb91e53c,
};

static uint32_t A[] = {
    0xff4be8c,  0x6f485986, 0x12843ff,  0x5b47dc4d, 0x7faa9b8a, 0xd547b8ba,
    0xf9979921, 0x4f5400da, 0x725f79a9, 0x3c9321ac, 0x32716d,   0x3f5adf5d,
};

Feature NTransformFeature::operator()(std::shared_ptr<Chunk> chunk) {
  uint8_t *content = chunk->buf();
  std::vector<uint32_t> sub_features(sf_cnt_ * sf_subf_, 0);
  std::vector<uint64_t> super_features(sf_cnt_, 0);
  const int chunk_length = chunk->len();
  const int features_num = sf_cnt_ * sf_subf_;

  rabin_t rabin_ctx;
  rabin_init(&rabin_ctx);
  for (int i = 0; i < chunk_length; i++) {
    rabin_append(&rabin_ctx, content[i]);
    const uint64_t finger_print = rabin_ctx.digest;
    for (int j = 0; j < features_num; j++) {
      const uint32_t transform = M[j] * finger_print + A[j];
      sub_features[j] = std::max(sub_features[j], transform);
    }
  }
  for (int i = 0; i < sf_cnt_; i++) {
    rabin_t rabin_ctx;
    rabin_init(&rabin_ctx);
    for (int j = 0; j < sf_subf_; j++) {
      auto sub_feature = sub_features[sf_subf_ * i + j];
      auto data_ptr = (uint8_t *)&sub_feature;
      for (int k = 0; k < sizeof(uint32_t); k++) {
        rabin_append(&rabin_ctx, data_ptr[k]);
      }
    }
    super_features[i] = rabin_ctx.digest;
  }
  return super_features;
}

Feature OdessFeature::operator()(std::shared_ptr<Chunk> chunk) {
  int features_num = sf_cnt_ * sf_subf_;
  std::vector<uint32_t> sub_features(features_num, 0);
  std::vector<uint64_t> super_features(sf_cnt_, 0);

  int chunk_length = chunk->len();
  uint8_t *content = chunk->buf();
  uint64_t finger_print = 0;
  // calculate sub features.
  for (int i = 0; i < chunk_length; i++) {
    finger_print = (finger_print << 1) + GEAR_TABLE[content[i]];
    if ((finger_print & mask_) == 0) {
      for (int j = 0; j < features_num; j++) {
        const uint32_t transform = (M[j] * finger_print + A[j]);
        // we need to guarantee that when sub_features[i] is not inited,
        // always set its value
        if (sub_features[j] >= transform || 0 == sub_features[j])
          sub_features[j] = transform;
      }
    }
  }

  // group sub features into super features.
  auto hash_buf = (const uint8_t *const)(sub_features.data());
  for (int i = 0; i < sf_cnt_; i++) {
    uint64_t hash_value = 0;
    auto this_hash_buf = hash_buf + i * sf_subf_ * sizeof(uint32_t);
    for (int j = 0; j < sf_subf_ * sizeof(uint32_t); j++) {
      hash_value = (hash_value << 1) + GEAR_TABLE[this_hash_buf[j]];
    }
    super_features[i] = hash_value;
  }
  return super_features;
}

Feature OdessSubfeatures::operator()(std::shared_ptr<Chunk> chunk) {
  int mask_ = default_odess_mask;
  int features_num = 12;
  std::vector<uint64_t> sub_features(features_num, 0);

  int chunk_length = chunk->len();
  uint8_t *content = chunk->buf();
  uint32_t finger_print = 0;
  // calculate sub features.
  for (int i = 0; i < chunk_length; i++) {
    finger_print = (finger_print << 1) + GEAR_TABLE[content[i]];
    if ((finger_print & mask_) == 0) {
      for (int j = 0; j < features_num; j++) {
        const uint64_t transform = (M[j] * finger_print + A[j]);
        // we need to guarantee that when sub_features[i] is not inited,
        // always set its value
        if (sub_features[j] >= transform || 0 == sub_features[j])
          sub_features[j] = transform;
      }
    }
  }

  return sub_features;
}

Feature PalantirFeature::operator()(std::shared_ptr<Chunk> chunk) {
  auto sub_features = std::get<std::vector<uint64_t>>(get_sub_features_(chunk));
  std::vector<std::vector<uint64_t>> results;

  auto group = [&](int sf_cnt, int sf_subf) -> std::vector<uint64_t> {
    std::vector<uint64_t> super_features(sf_cnt, 0);
    auto hash_buf = (const uint8_t *const)(sub_features.data());
    for (int i = 0; i < sf_cnt; i++) {
      uint64_t hash_value = 0;
      auto this_hash_buf = hash_buf + i * sf_subf * sizeof(uint64_t);
      for (int j = 4; j < sf_subf * sizeof(uint64_t); j++) {
        hash_value = (hash_value << 1) + GEAR_TABLE[this_hash_buf[j]];
      }
      super_features[i] = hash_value;
    }
  };

  results.push_back(group(3,4));
  results.push_back(group(4,3));
  results.push_back(group(6,2));
  return results;
}
} // namespace Delta