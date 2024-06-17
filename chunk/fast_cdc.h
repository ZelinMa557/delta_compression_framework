#pragma once
#include "chunk/chunker.h"
#include "utils/mapped_file.h"
namespace Delta {
class FastCDC : public Chunker {
public:
  FastCDC();
  bool ReinitWithFile(std::string file_name) override;
  std::shared_ptr<Chunk> GetNextChunk() override;

private:
  int min_chunk_size = 4096;
  int max_chunk_size = 8192 + 4096;
  int avg_chunk_size = 8192;
  uint64_t mask_s = (1LL << 12) - 1;
  uint64_t mask_l = (1LL << 14) - 1;
  

  mapped_file file;
  uint8_t *file_read_ptr = nullptr;
  uint64_t remaining_file_len = 0;
};
} // namespace Delta