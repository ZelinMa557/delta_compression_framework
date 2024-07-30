#include "recover_file.h"
#include "chunk/chunk.h"
namespace Delta {
void RecoverFile::RecoverSingle(const FileMeta &meta, const std::string &path,
                                Storage &storage) {
  std::ofstream out(path, std::ios::out);
  for (uint32_t i = meta.start_chunk_id; i <= meta.end_chunk_id; i++) {
    auto chunk = storage.GetChunkContent(i);
    out.write((char*)chunk->buf(), chunk->len());
  }
  out.close();
}
} // namespace Delta