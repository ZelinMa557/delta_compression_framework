// part of this file are migrated from
// https://github.com/dat-ecosystem-archive/rabin/
#include "chunk/chunk.h"
#include "chunk/rabin_cdc.h"
namespace delta {
namespace {
constexpr uint64_t min_chunk_size = 8192 - 2048;
constexpr uint64_t max_chunk_size = 8192 + 2028;
// the lower 13 bits are 1.
// see https://pdos.csail.mit.edu/papers/lbfs:sosp01/lbfs.pdf
constexpr uint64_t rabin_stop_mask = (1UL << 14) - 1;

constexpr uint64_t polynomial = 0x3DA3358B4DC173LL;
constexpr uint64_t polynomial_degree = 53;
constexpr uint64_t polynomial_shift = polynomial_degree - 8;
} // namespace

RabinCDC::RabinCDC() {
  auto deg = [&](uint64_t p) -> uint64_t {
    uint64_t mask = 0x8000000000000000LL;
    for (int i = 0; i < 64; i++) {
      if ((mask & p) > 0) {
        return 63 - i;
      }
      mask >>= 1;
    }
    return -1;
  };
  auto mod = [&](uint64_t x, uint64_t p) -> uint64_t {
    while (deg(x) >= deg(p)) {
      uint64_t shift = deg(x) - deg(p);
      x = x ^ (p << shift);
    }
    return x;
  };
  auto append_byte = [&](uint64_t hash, uint8_t b, uint64_t pol) -> uint64_t {
    hash <<= 8;
    hash |= (uint64_t)b;
    return mod(hash, pol);
  };

  // calculate table for sliding out bytes. The byte to slide out is used as
  // the index for the table, the value contains the following:
  // out_table[b] = Hash(b || 0 ||        ...        || 0)
  //                          \ windowsize-1 zero bytes /
  // To slide out byte b_0 for window size w with known hash
  // H := H(b_0 || ... || b_w), it is sufficient to add out_table[b_0]:
  //    H(b_0 || ... || b_w) + H(b_0 || 0 || ... || 0)
  //  = H(b_0 + b_0 || b_1 + 0 || ... || b_w + 0)
  //  = H(    0     || b_1 || ...     || b_w)
  //
  // Afterwards a new byte can be shifted in.
  for (int b = 0; b < 256; b++) {
    uint64_t hash = 0;

    hash = append_byte(hash, (uint8_t)b, polynomial);
    for (int i = 0; i < WINSIZE - 1; i++) {
      hash = append_byte(hash, 0, polynomial);
    }
    out_table[b] = hash;
  }

  // calculate table for reduction mod Polynomial
  int k = deg(polynomial);
  for (int b = 0; b < 256; b++) {
    // mod_table[b] = A | B, where A = (b(x) * x^k mod pol) and  B = b(x) * x^k
    //
    // The 8 bits above deg(Polynomial) determine what happens next and so
    // these bits are used as a lookup to this table. The value is split in
    // two parts: Part A contains the result of the modulus operation, part
    // B is used to cancel out the 8 top bits so that one XOR operation is
    // enough to reduce modulo Polynomial
    mod_table[b] = mod(((uint64_t)b) << k, polynomial) | ((uint64_t)b)
                                                                   << k;
  }
}

void RabinCDC::rabin_reset() {
  for (int i = 0; i < WINSIZE; i++)
    this->window[i] = 0;
  this->digest = 0;
  this->wpos = 0;
  this->digest = 0;
  rabin_slide(1);
}

void RabinCDC::rabin_slide(uint8_t b) {
  uint8_t out = this->window[this->wpos];
  this->window[this->wpos] = b;
  this->digest = (this->digest ^ out_table[out]);
  this->wpos = (this->wpos + 1) % WINSIZE;
  rabin_append(b);
}

void RabinCDC::rabin_append(uint8_t b) {
  uint8_t index = (uint8_t)(this->digest >> polynomial_shift);
  this->digest <<= 8;
  this->digest |= (uint64_t)b;
  this->digest ^= mod_table[index];
}

bool RabinCDC::ReinitWithFile(std::string file_name) {
  rabin_reset();
  this->file = std::move(mapped_file(file_name));
  if (!file.map_success()) {
    return false;
  }
  this->file_read_ptr = this->file.get_mapped_addr();
  this->remaining_file_len = this->file.get_maped_len();
  return true;
}

std::shared_ptr<Chunk> RabinCDC::GetNextChunk() {
  if (remaining_file_len == 0)
    return nullptr;
  if (remaining_file_len <= min_chunk_size) {
    remaining_file_len = 0;
    return Chunk::FromMemoryRef(file_read_ptr, remaining_file_len);
  }
  uint8_t *read_start = file_read_ptr;
  for (uint64_t chunk_size = 1; chunk_size <= remaining_file_len;
       chunk_size++) {
    uint8_t b = *file_read_ptr++;
    rabin_slide(b);
    if ((chunk_size >= min_chunk_size &&
         ((this->digest & rabin_stop_mask) == 0)) ||
        chunk_size >= max_chunk_size) {
      rabin_reset();
      remaining_file_len -= chunk_size;
      return Chunk::FromMemoryRef(read_start, chunk_size);
    }
  }
  return nullptr;
}
} // namespace delta