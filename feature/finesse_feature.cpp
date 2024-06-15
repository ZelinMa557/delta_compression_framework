#include "chunk/chunk.h"
#include "feature/features.h"
#include "utils/rabin.cpp"
#include <algorithm>
namespace delta {
std::vector<uint64_t> FinesseFeature(std::shared_ptr<Chunk> chunk,
                                     const int sf_cnt, const int sf_subf) {
  int sub_chunk_length = chunk->len() / sf_subf;
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
    for (int j = 0; j < sf_subf; i++) {
      super_features[j] = std::max(super_features[j], sub_features[i + j]);
    }
  }
  return super_features;
}
} // namespace delta