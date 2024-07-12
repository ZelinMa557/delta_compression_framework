#include "chunk/crc_cdc.h"
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
  crc_sigs.reserve(sub_chunk_count_);
  for (int i = 0; i < sub_chunk_count_; i++) {
    if (remaining_file_len < max_sub_chunk_size_) {
      total_size += remaining_file_len;
      crc_sigs.push_back(crc32c::Extend(0, file_read_ptr, remaining_file_len));
      remaining_file_len = 0;
      break;
    }
    int cur_sub_chk_size = min_sub_chunk_size_;
    uint32_t crc32_sig = crc32c::Extend(0, file_read_ptr, cur_sub_chk_size);
    for (; cur_sub_chk_size <= max_sub_chunk_size_;
         cur_sub_chk_size += CRC_BATCH_SIZE) {
      if (crc32_sig & stop_mask_ == 0)
        break;
      crc32_sig = crc32c::Extend(crc32_sig, file_read_ptr + cur_sub_chk_size,
                                 CRC_BATCH_SIZE);
    }
    remaining_file_len -= cur_sub_chk_size;
    file_read_ptr += cur_sub_chk_size;
    crc_sigs.push_back(crc32_sig);
    total_size += cur_sub_chk_size;
  }
  return CRC_Chunk::FromMemoryRef(chunk_start_pos, total_size,
                                  get_next_chunk_id(), crc_sigs);
}
} // namespace Delta