#pragma once
#include <string>
#include <memory>
#include <optional>
namespace Delta {
using chunk_id = uint32_t;
class Chunk;
class Index {
public:
    virtual std::optional<chunk_id> GetBaseChunkID(std::shared_ptr<Chunk> chunk, bool add_new_base_chunk) = 0;
    virtual bool RecoverFromFile(const std::string &path) = 0;
    virtual bool DumpToFile(const std::string &path) = 0;
};
} // namespace Delta