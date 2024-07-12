#pragma once
#include "chunk/chunk.h"
#include "chunk/chunker.h"
#include "utils/mapped_file.h"
#include <vector>
namespace Delta {

class CRC_Chunk : public Chunk {
public:
  CRC_Chunk(uint8_t *buf, bool needs_to_free, int length, chunk_id id,
            std::vector<uint32_t> &crc_sigs)
      : Chunk(buf, needs_to_free, length, id), crc_sigs_(std::move(crc_sigs)) {}
  const std::vector<uint32_t> &crc_sigs() const {}
  static std::shared_ptr<Chunk> FromMemory(void *start, size_t size,
                                           chunk_id id,
                                           std::vector<uint32_t> &crc_sigs) {
    auto buf = new uint8_t[size];
    memcpy(buf, start, size);
    auto crc_chunk = new CRC_Chunk(buf, true, size, id, crc_sigs);
    return std::shared_ptr<Chunk>(crc_chunk);
  }

  static std::shared_ptr<Chunk>
  FromFileStream(FILE *fp, size_t size, chunk_id id,
                 std::vector<uint32_t> &crc_sigs) {
    auto buf = new uint8_t[size];
    fread(buf, 1, size, fp);
    auto crc_chunk = new CRC_Chunk(buf, true, size, id, crc_sigs);
    return std::shared_ptr<Chunk>(crc_chunk);
  }

  static std::shared_ptr<Chunk>
  FromMemoryRef(void *start, size_t size, uint32_t id,
                const std::vector<uint32_t> &crc_sigs) {
    auto chunk = std::make_shared<CRC_Chunk>(static_cast<uint8_t *>(start),
                                             false, size, id, crc_sigs);
    return chunk;
  }

private:
  std::vector<uint32_t> crc_sigs_;
};

class CRC_CDC : public Chunker {
public:
  CRC_CDC(uint64_t min_sub_ck_sz, uint64_t max_sub_ck_sz, uint64_t mask,
          uint64_t sub_chunk_count)
      : min_sub_chunk_size_(min_sub_ck_sz), max_sub_chunk_size_(max_sub_ck_sz),
        stop_mask_(mask), sub_chunk_count_(sub_chunk_count) {}
  bool ReinitWithFile(std::string file_name) override;
  std::shared_ptr<Chunk> GetNextChunk() override;

private:
  mapped_file file;
  uint8_t *file_read_ptr = nullptr;
  uint64_t remaining_file_len = 0;
  uint64_t min_sub_chunk_size_;
  uint64_t max_sub_chunk_size_;
  uint64_t stop_mask_;
  uint64_t sub_chunk_count_;
};
} // namespace Delta