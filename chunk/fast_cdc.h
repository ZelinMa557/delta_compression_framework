#pragma once
#include "chunk/chunker.h"
#include "utils/mapped_file.h"
#include <cstdint>
namespace Delta {
class FastCDC : public Chunker {
public:
  FastCDC();
  bool ReinitWithFile(std::string file_name) override;
  std::shared_ptr<Chunk> GetNextChunk() override;

private:
  int min_chunk_size = 4096 + 2048;
  int max_chunk_size = 8192 + 1048;
  const uint64_t mask = (1LL << 13) - 1;
  

  mapped_file file;
  uint8_t *file_read_ptr = nullptr;
  uint64_t remaining_file_len = 0;
};
} // namespace Delta