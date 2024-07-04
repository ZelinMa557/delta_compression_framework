#include "chunk/chunk.h"
#include "storage/cache.h"
namespace Delta {
std::shared_ptr<Chunk> ChunkCache::get(uint32_t chunk_id) {
  if (!map_.count(chunk_id)) {
    cache_miss_++;
    return nullptr;
  }
  cache_hits_++;
  auto *node = map_[chunk_id];
  auto *prev = node->prev;
  auto *next = node->next;
  prev->next = next;
  next->prev = prev;
  insertToHead(node);
  return node->chunk;
}

void ChunkCache::add(uint32_t chunk_id, std::shared_ptr<Chunk> chunk) {
  while (current_memory + chunk->len() >= max_memory_usage) {
    auto *evict = dummy_tail->prev;
    auto *prev = evict->prev;
    prev->next = dummy_tail;
    dummy_tail->prev = prev;
    map_.erase(evict->chunk_id);
    delete evict;
    current_memory -= chunk->len();
  }
  current_memory += chunk->len();
  auto new_node = new Node;
  new_node->chunk_id = chunk_id;
  new_node->chunk = chunk;
  map_[chunk_id] = new_node;
  insertToHead(new_node);
}

void ChunkCache::insertToHead(ChunkCache::Node *node) {
  auto *head_next = dummy_head->next;
  dummy_head->next = node;
  node->prev = dummy_head;
  head_next->prev = node;
  node->next = head_next;
}
} // namespace Delta