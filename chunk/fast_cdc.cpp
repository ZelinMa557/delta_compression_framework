// reference https://github.com/jotfs/fastcdc-go/blob/master/fastcdc.go
#include "chunk/fast_cdc.h"
#include "chunk/chunk.h"
#include "utils/gear.h"
#include <cstdint>
#include <glog/logging.h>
namespace Delta {

bool FastCDC::ReinitWithFile(std::string file_name) {
  this->file = std::move(mapped_file(file_name));
  if (!file.map_success()) {
    return false;
  }
  this->file_read_ptr = this->file.get_mapped_addr();
  this->remaining_file_len = this->file.get_maped_len();
  LOG(INFO) << "FastCDC inited with file: " << file_name
            << " length: " << remaining_file_len;
  return true;
}

std::shared_ptr<Chunk> FastCDC::GetNextChunk() {
  if (remaining_file_len == 0) {
    return nullptr;
  }
  if (remaining_file_len <= min_chunk_size) {
    auto chunk_size = remaining_file_len;
    remaining_file_len = 0;
    LOG(INFO) << "FastCDC split file " << this->file.get_file_name() << " id "
              << next_chunk_id_ << " chunk offset: "
              << (uint64_t)(file_read_ptr - file.get_mapped_addr())
              << " length: " << chunk_size;
    return Chunk::FromMemoryRef(file_read_ptr, chunk_size, get_next_chunk_id());
  }
  uint64_t finger_print = 0;
  int chunk_size = 1;
  auto read_start = file_read_ptr;
  for (; chunk_size <= min_chunk_size; chunk_size++) {
    uint8_t byte = *file_read_ptr++;
    finger_print = (finger_print << 1) + GEAR_TABLE[byte];
  }
  for (; chunk_size <=
         std::min((uint64_t)remaining_file_len, (uint64_t)max_chunk_size);
       chunk_size++) {
    uint8_t byte = *file_read_ptr++;
    finger_print = (finger_print << 1) + GEAR_TABLE[byte];
    if ((finger_print & mask) == 0) {
      remaining_file_len -= chunk_size;
      LOG(INFO) << "FastCDC split file " << this->file.get_file_name() << " id "
                << next_chunk_id_ << " chunk offset: "
                << (uint64_t)(read_start - file.get_mapped_addr())
                << " length: " << chunk_size;
      return Chunk::FromMemoryRef(read_start, chunk_size, get_next_chunk_id());
    }
  }
  chunk_size--;
  LOG(INFO) << "FastCDC split file " << this->file.get_file_name() << " id "
            << next_chunk_id_ << " chunk offset: "
            << (uint64_t)(read_start - file.get_mapped_addr())
            << " length: " << chunk_size;
  remaining_file_len -= chunk_size;
  return Chunk::FromMemoryRef(read_start, chunk_size, get_next_chunk_id());
}

} // namespace Delta