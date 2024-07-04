#pragma once
#include <iomanip>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <vector>
namespace Delta {
class Chunk;
class ChunkCache {
public:
  ChunkCache(size_t cache_size) : max_memory_usage(cache_size * 1024 * 1024) {
    dummy_head = new Node;
    dummy_tail = new Node;
    dummy_head->next = dummy_tail;
    dummy_tail->prev = dummy_head;
  }
  ~ChunkCache() {
    auto print_ratio = [](uint32_t a, uint32_t b) {
      double ratio = (double)a / (double)b;
      std::cout << std::fixed << std::setprecision(1);
      std::cout << ratio * 100 << "%" << std::endl;
      std::cout << std::defaultfloat;
    };
    std::cout << "Storage::ChunkCache: cache hits ratio: ";
    print_ratio(cache_hits_, cache_hits_ + cache_miss_);
    std::cout << std::endl;
  }
  struct Node {
    Node *prev = nullptr;
    Node *next = nullptr;
    uint32_t chunk_id;
    std::shared_ptr<Chunk> chunk;
  };

  std::shared_ptr<Chunk> get(uint32_t chunk_id);
  void add(uint32_t chunk_id, std::shared_ptr<Chunk> chunk);

private:
  void insertToHead(Node *node);
  const size_t max_memory_usage;
  size_t current_memory = 0;
  std::unordered_map<uint32_t, Node *> map_;
  Node *dummy_head;
  Node *dummy_tail;
  uint32_t cache_hits_ = 0;
  uint32_t cache_miss_ = 0;
};
} // namespace Delta