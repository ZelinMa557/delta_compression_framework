#include <unordered_map>
#include <string>
#include "utils/sha1.h"
#include "chunk/chunk.h"
namespace Delta {
class Dedup {
public:
  ~Dedup() { Dump(); }
  Dedup(const std::string &path) : file_path_(path) {}
  // return the base chunk id
  uint32_t ProcessChunk(std::shared_ptr<Chunk> &chunk) {
    auto digest = sha1_hash(chunk->buf(), chunk->len());
    if (mp_.count(digest)) {
      return mp_[digest];
    }
    mp_[digest] = chunk->id();
    return chunk->id();
  }

  void Dump() {
    //todo
  }

  void Recover() {
    //todo
  }
private:
  std::unordered_map<SHA1_digest, uint32_t> mp_;
  std::string file_path_;
};
}