#pragma once
#include "chunk/chunker.h"
#include "utils/mapped_file.h"
#include <cstdint>
namespace Delta {
class FastCDC : public Chunker {
public:
  FastCDC(int min_ck_sz, int max_ck_sz, uint64_t stop_mask)
      : min_chunk_size(min_ck_sz), max_chunk_size(max_ck_sz), mask(stop_mask) {}
  bool ReinitWithFile(std::string file_name) override;
  std::shared_ptr<Chunk> GetNextChunk() override;

private:
  const int min_chunk_size;
  const int max_chunk_size;
  const uint64_t mask;

  mapped_file file;
  uint8_t *file_read_ptr = nullptr;
  uint64_t remaining_file_len = 0;
};
} // namespace Delta