#pragma once
#include "encoder/encoder.h"
#include "storage/cache.h"
#include <cstdio>
#include <memory>
#include <string>
namespace Delta {
constexpr uint8_t BaseChunk = 1;
constexpr uint8_t DeltaChunk = 2;
constexpr uint8_t DuplicateChunk = 3;
using chunk_id = uint32_t;
class Chunk;
struct ChunkMeta {
  uint64_t offset;
  chunk_id base_chunk_id;
  uint16_t size;
  uint8_t type;
} __attribute__((aligned(16)));

class Storage {
public:
  Storage(std::string DataPath, std::string MetaPath,
          std::unique_ptr<Encoder> encoder, bool compress_mode,
          size_t cache_size);
  ~Storage() {
    if (data_)
      fclose(data_);
    if (meta_)
      fclose(meta_);
  }
  void WriteBaseChunk(std::shared_ptr<Chunk> chunk);
  int WriteDeltaChunk(std::shared_ptr<Chunk> chunk, chunk_id base_chunk_id);
  void WriteDuplicateChunk(std::shared_ptr<Chunk> chunk,
                           chunk_id base_chunk_id);
  std::shared_ptr<Chunk> GetChunkContent(chunk_id id);
  std::shared_ptr<Chunk> GetDeltaEncodedChunk(std::shared_ptr<Chunk> chunk,
                                              chunk_id base_chunk_id);

private:
  std::unique_ptr<Encoder> encoder_;
  ChunkCache cache_;
  FILE *data_ = nullptr;
  FILE *meta_ = nullptr;
};
} // namespace Delta