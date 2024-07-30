#include "utils/simhash.h"

#include <cstring>
#include <immintrin.h>
#include <xmmintrin.h>

namespace simhash {

hash_t simhash(const std::vector<hash_t>& hashes) {
  hash_t result = 0;
  // Initialize counts to 0
  std::vector<long> counts(BITS, 0);

  // Count the number of 1's, 0's in each position of the hashes
  for (hash_t hash : hashes) {
    for (size_t i = 0; i < BITS; ++i) {
      counts[i] += (hash & 1) ? 1 : -1;
      hash >>= 1;
    }
  }

  // Produce the result
  for (size_t i = 0; i < BITS; ++i) {
    if (counts[i] > 0) {
      result |= (static_cast<hash_t>(1) << i);
    }
  }
  return result;
}

hash_t simhash2(const std::vector<hash_t>& hashes) {
  hash_t result = 0;
  // Initialize counts to 0
  std::vector<long> counts(BITS, 0);

  auto it = hashes.begin();
  auto first_hash = *it++;
  for (size_t i = 0; i < BITS; ++i) {
    counts[i] += (first_hash & 1) ? 1 : -1;
    first_hash >>= 1;
  }
  auto last_hash = first_hash;

  // Count the number of 1's, 0's in each position of the hashes
  for (; it < hashes.end(); ++it) {
    auto hash = *it;
    for (size_t i = 0; i < BITS; ++i) {
      if (last_hash ^ hash) {
        counts[i] += (hash & 1) ? 2 : -2;
      } else {
        counts[i] += (hash & 1) ? 1 : -1;
      }
      // counts[i] += (hash & 1) ? ((last_hash & hash & 1) ? 3 : 2) : -3;
      hash >>= 1;
      last_hash >>= 1;
    }
    last_hash = *it;
  }

  // Produce the result
  for (size_t i = 0; i < BITS; ++i) {
    if (counts[i] > 0) {
      result |= (static_cast<hash_t>(1) << i);
    }
  }
  return result;
}

uint64_t simhash_ex(const std::vector<hash_t>& hashes) {
  hash_t result = 0;
  // Initialize counts to 0
  std::vector<long> counts(BITS, 0);

  auto it = hashes.begin();
  auto first_hash = *it++;
  for (size_t i = 0; i < BITS; ++i) {
    counts[i] += (first_hash & 1) ? 1 : -1;
    first_hash >>= 1;
  }
  auto last_hash = first_hash;

  // Count the number of 1's, 0's in each position of the hashes
  for (; it < hashes.end(); ++it) {
    auto hash = *it;
    for (size_t i = 0; i < BITS; ++i) {
      if (last_hash ^ hash) {
        counts[i] += (hash & 1) ? 2 : -2;
      } else {
        counts[i] += (hash & 1) ? 1 : -1;
      }
      // counts[i] += (hash & 1) ? ((last_hash & hash & 1) ? 3 : 2) : -3;
      hash >>= 1;
      last_hash >>= 1;
    }
    last_hash = *it;
  }

  // Produce the result
  for (size_t i = 0; i < 64; i += 2) {
    uint64_t val = 0;
    if (counts[i] > 1) {
      val = 0b11;
    } else if (counts[i] > 0) {
      val = 0b01;
    }
    result |= (val << i);
  }
  return result;
}

uint64_t simhash_ex(uint8_t *content, size_t size) {
  hash_t result = 0;
  // Initialize counts to 0
  const int BIT = 8;
  std::vector<long> counts(BIT, 0);

  auto first_hash = content[0];
  for (size_t i = 0; i < BIT; ++i) {
    counts[i] += (first_hash & 1) ? 1 : -1;
    first_hash >>= 1;
  }
  auto last_hash = first_hash;

  // Count the number of 1's, 0's in each position of the hashes
  for (int i = 1; i < size; ++i) {
    auto hash = content[i];
    for (size_t i = 0; i < BIT; ++i) {
      if (last_hash ^ hash) {
        counts[i] += (hash & 1) ? 2 : -2;
      } else {
        counts[i] += (hash & 1) ? 1 : -1;
      }
      // counts[i] += (hash & 1) ? ((last_hash & hash & 1) ? 3 : 2) : -3;
      hash >>= 1;
      last_hash >>= 1;
    }
    last_hash = content[i];
  }

  // Produce the result
  for (size_t i = 0; i < 64; i += 2) {
    uint64_t val = 0;
    if (counts[i] > 1) {
      val = 0b11;
    } else if (counts[i] > 0) {
      val = 0b01;
    }
    result |= (val << i);
  }
  return result;
}

#ifdef __AVX2__
hash_t simhash_avx2(const hash_t* hashes, const uint32_t hash_num) {
  // Initialize counts to 0
  alignas(32) int32_t counts[BITS]{0};
  const hash_t* p_hashes = hashes;

  for (uint32_t i = 0; i < hash_num; ++i) {
    hash_t hash = *p_hashes++;
    auto* p_counts = reinterpret_cast<__m256i*>(counts);

    for (uint32_t j = 0; j < (BITS >> 3); ++j) {
      auto bitsSet = _mm256_load_si256(
          reinterpret_cast<const __m256i*>(&(lookup[hash & 0xff])));
      _mm256_store_si256(p_counts, _mm256_add_epi32(*p_counts, bitsSet));
      ++p_counts;
      hash >>= 8;
    }
  }

  // Produce the result
  hash_t result = 0;
  auto* p_counts = reinterpret_cast<__m256i*>(counts);
  for (size_t i = 0; i < BITS; i += 8) {
    __m256i cmp_result = _mm256_cmpgt_epi32(*p_counts, _mm256_setzero_si256());
    result |= (_mm256_movemask_ps(_mm256_castsi256_ps(cmp_result)) << i);
    ++p_counts;
  }
  return result;
}

bool SimHash::is_lookup2_initialized = false;
bool SimHash::is_lookup_16b_initialized = false;
alignas(32) int32_t SimHash::lookup2[256][256][8];
alignas(32) int16_t SimHash::lookup_16b[65536][16];

void* SimHash::LoopUp2(const uint32_t and_value, const uint32_t hash_byte) {
  if (!is_lookup2_initialized) {
    is_lookup2_initialized = true;
    for (int i = 0; i < 256; ++i) {
      for (int j = 0; j < 256; ++j) {
        for (int n = 0; n < 8; ++n) {
          if (((i >> n) & 1)) {
            SimHash::lookup2[i][j][n] = 2;
          } else {
            SimHash::lookup2[i][j][n] = (((j >> n) & 1) << 1) - 1;
          }
        }
      }
    }
  }
  return SimHash::lookup2[and_value][hash_byte];
}

void* SimHash::LoopUp16b(uint32_t hash_byte) {
  if (!is_lookup_16b_initialized) {
    is_lookup_16b_initialized = true;
    for (int i = 0; i < 65536; ++i) {
      for (int n = 0; n < 16; ++n) {
        SimHash::lookup_16b[i][n] = (((i >> n) & 1) << 1) - 1;
      }
    }
  }

  return SimHash::lookup_16b[hash_byte];
}

hash_t simhash_avx2_16b(const hash_t* hashes, const uint32_t hash_num) {
  // Initialize counts to 0
  alignas(32) int16_t counts[BITS]{0};
  const hash_t* p_hashes = hashes;

  for (uint32_t i = 0; i < hash_num; ++i) {
    hash_t hash = *p_hashes++;
    auto* p_counts = reinterpret_cast<__m256i*>(counts);

    for (uint32_t j = 0; j < (BITS >> 4); ++j) {
      auto bitsSet = _mm256_load_si256(
          reinterpret_cast<const __m256i*>(SimHash::LoopUp16b(hash & 0xffff)));
      _mm256_store_si256(p_counts, _mm256_add_epi16(*p_counts, bitsSet));
      ++p_counts;
      hash >>= 16;
    }
  }

  // Produce the result
  hash_t result = 0;
  auto* p_counts = reinterpret_cast<__m256i*>(counts);
  for (size_t i = 0; i < BITS; i += 16) {
    __m256i cmp_result = _mm256_cmpgt_epi16(*p_counts, _mm256_setzero_si256());
    __m128i packed_high_bits =
        _mm_packs_epi16(_mm256_castsi256_si128(cmp_result),
                        _mm256_extracti128_si256(cmp_result, 1));
    int half = _mm_movemask_epi8(packed_high_bits);
    result |= (half << i);
    ++p_counts;
  }
  return result;
}

hash_t simhash_avx2v2(const hash_t* hashes, const uint32_t hash_num) {
  // Initialize counts to 0
  alignas(32) int32_t counts[BITS]{0};
  const hash_t* p_hashes = hashes;

  hash_t last_hash = 0;

  for (uint32_t i = 0; i < hash_num; ++i) {
    hash_t hash = *p_hashes;
    auto* p_counts = reinterpret_cast<__m256i*>(counts);
    hash_t and_value = last_hash & hash;

    for (uint32_t j = 0; j < (BITS >> 3); ++j) {
      auto bitsSet = _mm256_load_si256(reinterpret_cast<const __m256i*>(
          SimHash::LoopUp2(and_value & 0xff, hash & 0xff)));
      _mm256_store_si256(p_counts, _mm256_add_epi32(*p_counts, bitsSet));
      ++p_counts;
      hash >>= 8;
      and_value >>= 8;
    }
    last_hash = *p_hashes++;
  }

  // Produce the result
  hash_t result = 0;
  auto* p_counts = reinterpret_cast<__m256i*>(counts);
  for (size_t i = 0; i < BITS; i += 8) {
    __m256i cmp_result = _mm256_cmpgt_epi32(*p_counts, _mm256_setzero_si256());
    result |= (_mm256_movemask_ps(_mm256_castsi256_ps(cmp_result)) << i);
    ++p_counts;
  }
  return result;
}

#endif
}  // namespace simhash