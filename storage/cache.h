#pragma once
#include <memory>
#include <unordered_map>
#include <vector>
namespace Delta {
class Chunk;
class ChunkCache {
public:
  struct Node {
    Node *prev = nullptr;
    Node *next = nullptr;
    uint32_t chunk_id;
    std::shared_ptr<Chunk> chunk;
  };
  ChunkCache() {
    dummy_head = new Node;
    dummy_tail = new Node;
    dummy_head->next = dummy_tail;
    dummy_tail->prev = dummy_head;
  }
  std::shared_ptr<Chunk> get(uint32_t chunk_id);
  void add(uint32_t chunk_id, std::shared_ptr<Chunk> chunk);

private:
  void insertToHead(Node *node);
  const size_t max_memory_usage = 100 * 1024 * 1024;
  size_t current_memory = 0;
  std::unordered_map<uint32_t, Node *> map_;
  Node *dummy_head;
  Node *dummy_tail;
};
} // namespace Delta