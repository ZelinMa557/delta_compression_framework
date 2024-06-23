// part of this file migrate from
// https://github.com/dat-ecosystem-archive/rabin/
#pragma once
#include "chunk/chunker.h"
#include "utils/mapped_file.h"
namespace Delta {
class RabinCDC : public Chunker {
public:
  RabinCDC(uint64_t min_ck_sz, uint64_t max_ck_sz, uint64_t mask);
  bool ReinitWithFile(std::string file_name) override;
  std::shared_ptr<Chunk> GetNextChunk() override;

private:
  uint64_t mod_table[256];
  uint64_t out_table[256];
  // window size 48 is recommanded by https://pdos.csail.mit.edu/papers/lbfs:sosp01/lbfs.pdf
  static const int WINSIZE = 48;
  uint8_t window[WINSIZE];
  uint64_t wpos;
  uint64_t digest;

  void rabin_reset();
  void rabin_slide(uint8_t b);
  void rabin_append(uint8_t b);

  mapped_file file;
  uint8_t *file_read_ptr = nullptr;
  uint64_t remaining_file_len = 0;
  uint64_t min_chunk_size_;
  uint64_t max_chunk_size_;
  uint64_t rabin_stop_mask_;
};
} // namespace Delta