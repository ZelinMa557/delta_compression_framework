#include "chunk/chunk.h"
#include "feature/features.h"
#include "utils/rabin.cpp"
#include <algorithm>
namespace Delta {
std::vector<uint64_t> FinesseFeature(std::shared_ptr<Chunk> chunk,
                                     const int sf_cnt, const int sf_subf) {
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
      auto data_ptr = (uint8_t*)&sub_feature;
      for (int k = 0; k < 8; k++) {
        rabin_append(&rabin_ctx, data_ptr[k]);
      }
    }
    super_features[i] = rabin_ctx.digest;
  }
  return super_features;
}
} // namespace Delta