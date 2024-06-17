#include "storage/storage.h"
#include <cstdlib>
#include <iostream>
namespace Delta {
Storage::Storage(std::string DataPath, std::string MetaPath,
                 std::shared_ptr<Encoder> encoder, bool compress_mode) {
  this->encoder_ = encoder;
  char fopen_flag[3] = {'r', '+', '\0'};
  if (compress_mode) {
    fopen_flag[0] = 'w';
  }
  data_ = fopen(DataPath.c_str(), fopen_flag);
  meta_ = fopen(MetaPath.c_str(), fopen_flag);
  if (!data_ || !meta_) {
    std::cerr << "Fail to open " << DataPath << " or " << MetaPath << std::endl;
    std::abort();
  }
  fseek(data_, 0, SEEK_END);
  fseek(meta_, 0, SEEK_END);
}

void Storage::WriteBaseChunk(std::shared_ptr<Chunk> chunk) {
  ChunkMeta meta;
  meta.offset = ftell(data_);
  meta.base_chunk_id = chunk.id();
  meta.size = chunk.len();
  meta.type = BaseChunk;

  fwrite(data_, 1, chunk.len(), chunk.buf());
  fwrite(meta_, sizeof(ChunkMeta), 1, &meta);
}

void Storage::WriteDeltaChunk(std::shared_ptr<Chunk> chunk,
                              chunk_id base_chunk_id) {
  auto base_chunk = cache_->get(base_chunk_id);
  if (nullptr == base_chunk) {
    base_chunk = GetChunkContent(base_chunk_id);
    cache_->add(base_chunk_id, base_chunk);
  }
  auto delta_chunk = encoder_->encode(chunk, base_chunk);
  ChunkMeta meta;
  meta.offset = ftell(data_);
  meta.base_chunk_id = base_chunk_id;
  meta.size = delta_chunk.len();
  meta.type = DeltaChunk;

  fwrite(data_, 1, delta_chunk.len(), delta_chunk.buf());
  fwrite(meta_, sizeof(ChunkMeta), 1, &meta);
}

std::shared_ptr<Chunk> Storage::GetChunkContent(chunk_id id) {
  // TODO: support recover data content of delta chunk
  uint64_t meta_offset = id * sizeof(ChunkMeta);
  fseek(meta_, meta_offset, SEEK_SET);
  ChunkMeta meta;
  fread(&meta, sizeof(ChunkMeta), 1, meta_);
  fseek(data_, meta.offset, SEEK_SET);
  result = Chunk::FromFileStream(data_, meta.size, id);
  fseek(meta_, 0, SEEK_END);
  fseek(data_, 0, SEEK_END);
  return result;
}
} // namespace Delta