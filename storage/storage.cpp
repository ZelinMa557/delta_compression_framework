#include "storage/storage.h"
#include "chunk/chunk.h"
#include <cstdlib>
#include <glog/logging.h>
namespace Delta {
Storage::Storage(std::string DataPath, std::string MetaPath,
                 std::unique_ptr<Encoder> encoder, bool compress_mode,
                 size_t cache_size)
    : cache_(cache_size) {
  this->encoder_ = std::move(encoder);
  char fopen_flag[3] = {'r', '+', '\0'};
  if (compress_mode) {
    fopen_flag[0] = 'w';
  }
  data_ = fopen(DataPath.c_str(), fopen_flag);
  meta_ = fopen(MetaPath.c_str(), fopen_flag);
  if (!data_ || !meta_) {
    LOG(FATAL) << "Fail to open " << DataPath << " or " << MetaPath;
  }
  fseek(data_, 0, SEEK_END);
  fseek(meta_, 0, SEEK_END);
}

void Storage::WriteBaseChunk(std::shared_ptr<Chunk> chunk) {
  ChunkMeta meta;
  meta.offset = ftell(data_);
  meta.base_chunk_id = chunk->id();
  meta.size = chunk->len();
  meta.type = BaseChunk;

  fwrite(chunk->buf(), 1, chunk->len(), data_);
  fwrite(&meta, sizeof(ChunkMeta), 1, meta_);
  LOG(INFO) << "write base chunk " << chunk->id() << ", len " << chunk->len();
}

std::shared_ptr<Chunk>
Storage::GetDeltaEncodedChunk(std::shared_ptr<Chunk> chunk,
                              chunk_id base_chunk_id) {
  auto base_chunk = cache_.get(base_chunk_id);
  if (nullptr == base_chunk) {
    base_chunk = GetChunkContent(base_chunk_id);
    cache_.add(base_chunk_id, base_chunk);
  }
  auto delta_chunk = encoder_->encode(base_chunk, chunk);
  return delta_chunk;
}

int Storage::WriteDeltaChunk(std::shared_ptr<Chunk> delta_chunk,
                             chunk_id base_chunk_id) {
  ChunkMeta meta;
  meta.offset = ftell(data_);
  meta.base_chunk_id = base_chunk_id;
  meta.size = delta_chunk->len();
  meta.type = DeltaChunk;

  fwrite(delta_chunk->buf(), 1, delta_chunk->len(), data_);
  fwrite(&meta, sizeof(ChunkMeta), 1, meta_);
  LOG(INFO) << "write delta chunk " << delta_chunk->id() << ", len "
            << delta_chunk->len();
  return delta_chunk->len();
}

std::shared_ptr<Chunk> Storage::GetChunkContent(chunk_id id) {
  // TODO: support recover data content of delta chunk
  uint64_t meta_offset = id * sizeof(ChunkMeta);
  fseek(meta_, meta_offset, SEEK_SET);
  ChunkMeta meta;
  fread(&meta, sizeof(ChunkMeta), 1, meta_);
  fseek(data_, meta.offset, SEEK_SET);
  auto raw_content = Chunk::FromFileStream(data_, meta.size, id);
  fseek(meta_, 0, SEEK_END);
  fseek(data_, 0, SEEK_END);
  if (meta.type == DeltaChunk) {
    auto base_chunk = cache_.get(meta.base_chunk_id);
    if (nullptr == base_chunk) {
      base_chunk = GetChunkContent(meta.base_chunk_id);
      cache_.add(meta.base_chunk_id, base_chunk);
    }
    return encoder_->decode(base_chunk, raw_content);
  }
  return raw_content;
}

void Storage::WriteDuplicateChunk(std::shared_ptr<Chunk> chunk,
                                  chunk_id base_chunk_id) {
  ChunkMeta meta;
  meta.base_chunk_id = base_chunk_id;
  meta.size = chunk->len();
  meta.type = DuplicateChunk;
  fseek(meta_, base_chunk_id & sizeof(ChunkMeta), SEEK_SET);
  ChunkMeta base_meta;
  fread(&base_meta, sizeof(ChunkMeta), 1, meta_);
  meta.offset = base_meta.offset;
  fseek(meta_, 0, SEEK_END);
  fwrite(&meta, sizeof(ChunkMeta), 1, meta_);
  LOG(INFO) << "write duplicate chunk " << chunk->id() << ", base id "
            << base_chunk_id;
}
} // namespace Delta