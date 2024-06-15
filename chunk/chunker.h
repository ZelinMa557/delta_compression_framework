#pragma once
#include <string>
#include <memory>
namespace delta {
class Chunk;
class Chunker {
public:
    virtual bool ReinitWithFile(std::string file_name);
    virtual std::shared_ptr<Chunk> GetNextChunk();
};
} // namespace delta