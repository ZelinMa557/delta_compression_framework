#include "chunk/crc_cdc.h"
#include "chunk/chunk.h"
#include <crc32c/crc32c.h>
#include <glog/logging.h>
namespace Delta {
constexpr int CRC_BATCH_SIZE = 4;
bool CRC_CDC::ReinitWithFile(std::string file_name) {
  this->file = std::move(mapped_file(file_name));
  if (!file.map_success()) {
    return false;
  }
  this->file_read_ptr = this->file.get_mapped_addr();
  this->remaining_file_len = this->file.get_maped_len();
  LOG(INFO) << "CRC CDC inited with file: " << file_name
            << " length: " << remaining_file_len;
  return true;
}

std::shared_ptr<Chunk> CRC_CDC::GetNextChunk() {
  if (remaining_file_len == 0) {
    return nullptr;
  }
  std::vector<uint32_t> crc_sigs;
  int total_size = 0;
  uint8_t *chunk_start_pos = file_read_ptr;
  auto chunk_size = std::min(min_chunk_size_, remaining_file_len);
  auto crc_sig = crc32c::Extend(0, file_read_ptr, chunk_size);
  for (; chunk_size < std::min(max_chunk_size_, remaining_file_len); chunk_size++) {
    if ((crc_sig & stop_mask_) == 0)
      break;
    crc_sig = crc32c::Extend(crc_sig, file_read_ptr + chunk_size, 1);
  }
  remaining_file_len -= chunk_size;
  file_read_ptr += chunk_size;
  return Chunk::FromMemoryRef(chunk_start_pos, chunk_size, get_next_chunk_id());
}
} // namespace Delta