#pragma once

#include <array>
#include <unordered_map>
#include <cstdint>

struct SHA1_digest {
  unsigned char d_[20];

  bool operator==(const SHA1_digest &other) const {
    return std::equal(d_, d_ + 20, other.d_);
  }
};

SHA1_digest sha1_hash(uint8_t *data, uint64_t length);

namespace std {
template <> struct hash<SHA1_digest> {
  size_t operator()(const SHA1_digest &x) const {
    size_t result = 0;
    for (int i = 0; i < 20; ++i) {
      result = result * 31 + x.d_[i];
    }
    return result;
  }
};
} // namespace std