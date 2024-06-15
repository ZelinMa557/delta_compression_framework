#pragma once
#include "chunk/chunker.h"
#include "utils/mapped_file.h"
namespace delta {
class FastCDC : public Chunker {
public:
  FastCDC();
  bool ReinitWithFile(std::string file_name) override;
  std::shared_ptr<Chunk> GetNextChunk() override;

private:
  

  mapped_file file;
  uint8_t *file_read_ptr = nullptr;
  uint64_t remaining_file_len = 0;
};
} // namespace delta