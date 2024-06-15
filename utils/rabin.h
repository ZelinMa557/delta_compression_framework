// modified from https://github.com/dat-ecosystem-archive/rabin/blob/master/src/rabin.h
#pragma once
#include <cstdint>
constexpr int WINSIZE = 48;

struct rabin_t {
  uint8_t window[WINSIZE];
  uint64_t wpos;
  uint64_t digest;
  uint64_t polynomial = 0x3DA3358B4DC173LL;
  uint64_t polynomial_degree = 53;
  uint64_t polynomial_shift = 53 - 8;
  uint64_t mask = (1UL << 14) - 1;
};

void rabin_init(struct rabin_t *h);
void rabin_slide(struct rabin_t *h, uint8_t b);