#include <vector>
#include <unordered_map>
#include <memory>
namespace Delta {
class Chunk;
class ChunkCache {
public:
    std::shared_ptr<Chunk> get(uint32_t chunk_id);
    void add (uint32_t chunk_id, std::shared_ptr<Chunk> chunk);
};
}