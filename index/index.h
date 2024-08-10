#pragma once
#include <string>
#include <memory>
#include <optional>
#include "feature/features.h"
namespace Delta {
using chunk_id = uint32_t;
class Chunk;
class Index {
public:
    virtual std::optional<chunk_id> GetBaseChunkID(const Feature &feat) = 0;
    virtual void AddFeature(const Feature &feat, chunk_id id) = 0;
    virtual bool RecoverFromFile(const std::string &path) = 0;
    virtual bool DumpToFile(const std::string &path) = 0;
};
} // namespace Delta