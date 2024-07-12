#pragma once
#include "chunk/chunker.h"
#include "utils/mapped_file.h"
#include <vector>
namespace Delta {

class CRC_CDC : public Chunker {
public:
  CRC_CDC(uint64_t min_ck_sz, uint64_t max_ck_sz, uint64_t mask)
      : min_chunk_size_(min_ck_sz), max_chunk_size_(max_ck_sz),
        stop_mask_(mask) {}
  bool ReinitWithFile(std::string file_name) override;
  std::shared_ptr<Chunk> GetNextChunk() override;

private:
  mapped_file file;
  uint8_t *file_read_ptr = nullptr;
  uint64_t remaining_file_len = 0;
  uint64_t min_chunk_size_;
  uint64_t max_chunk_size_;
  uint64_t stop_mask_;
};
} // namespace Delta