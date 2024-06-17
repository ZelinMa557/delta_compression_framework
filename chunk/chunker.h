#pragma once
#include <string>
#include <memory>
namespace Delta {
class Chunk;
class Chunker {
public:
    virtual bool ReinitWithFile(std::string file_name);
    virtual std::shared_ptr<Chunk> GetNextChunk();
protected:
    uint32_t get_next_chunk_id() {
        return next_chunk_id_++;
    }
    uint32_t next_chunk_id_;
};
} // namespace Delta