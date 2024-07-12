#pragma once
#include <cstdint>
inline uint64_t hammingDistance(uint64_t a, uint64_t b) {
    uint64_t xorResult = a ^ b;
    return __builtin_popcountll(xorResult);
}