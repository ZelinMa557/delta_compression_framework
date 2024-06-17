#pragma once
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
namespace Delta {
using chunk_id = uint32_t;
class Chunk {
public:
  Chunk() = delete;
  Chunk(uint8_t *buf, bool needs_to_free, int length, chunk_id id)
      : buf_(buf), needs_to_free_(needs_to_free), length_(length), id_(id) {};
  ~Chunk() {
    if (buf_ != nullptr && needs_to_free_)
      delete[] buf_;
  }
  static std::shared_ptr<Chunk> FromMemory(void *start, size_t size, uint32_t id) {
    auto buf = new uint8_t[size];
    memcpy(buf, start, size);
    return std::make_shared<Chunk>(buf, true, size, id);
  }
  int len() const { return length_; }
  uint8_t *buf() const { return buf_; }
  chunk_id id() const { return id_; }
  /*
   * look carefully to the life cycle of the buffer,
   * this function only record the address and length of the chunk content,
   * if you want to make a copy,
   * use FromMemory instead.
   */
  static std::shared_ptr<Chunk> FromMemoryRef(void *start, size_t size, uint32_t id) {
    return std::make_shared<Chunk>((uint8_t*)start, false, size, id);
  }
  // static std::shared_ptr<Chunk> FromFile(std::string file_name) {
  //     std::ifstream in(file_name, std::ios::binary);
  //     auto buf = new uint8_t[in.size];
  //     in.read(buf, in.size);
  //     return std::make_shared<Chunk>(buf);
  // }
  // static std::shared_ptr<Chunk> FromFile(std::string file_name, size_t
  // start_pos, size_t size) {
  //     std::ifstream in(file_name, std::ios::binary);
  //     auto buf = new uint8_t[in.size];
  //     in.seek(start_pos, std::ios::beg);
  //     in.read(buf, in.size);
  //     return std::make_shared<Chunk>(buf);
  // }

private:
  uint8_t *buf_ = nullptr;
  bool needs_to_free_ = false;
  int length_;
  chunk_id id_;
};
} // namespace Delta