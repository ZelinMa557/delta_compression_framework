#include <string>
#include <memory>
#include <cstdio>
#include "encoder/encoder.h"
#include "storage/cache.h"
namespace Delta {
constexpr uint8_t BaseChunk = 1;
constexpr uint8_t DeltaChunk = 2;
using chunk_id = uint32_t;
class Chunk;
struct ChunkMeta {
  uint64_t offset;
  chunk_id base_chunk_id;
  uint8_t type;
} __attribute__((aligned(128)));

class Storage {
public:
  Storage(std::string DataPath, std::string MetaPath, std::shared_ptr<Encoder> encoder);
  void WriteBaseChunk(std::shared_ptr<Chunk> chunk);
  void WriteDeltaChunk(std::shared_ptr<Chunk> chunk, chunk_id base_chunk_id);
private:
  std::shared_ptr<Encoder> encoder_;
  ChunkCache cache_;
  FILE *data_;
  FILE *meta_;
};
} // namespace Delta