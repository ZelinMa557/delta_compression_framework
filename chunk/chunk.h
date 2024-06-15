#pragma once
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
namespace delta {
class Chunk {
public:
  Chunk() = delete;
  Chunk(unsigned char *buf, bool needs_to_free, int length)
      : buf_(buf), needs_to_free_(needs_to_free), length_(length){};
  ~Chunk() {
    if (buf_ != nullptr && needs_to_free)
      delete[] buf_;
  }
  static std::shared_ptr<Chunk> FromMemory(void *start, size_t size) {
    auto buf = new unsigned char[size];
    memcpy(buf, start, size);
    return std::make_shared<Chunk>(buf, true);
  }
  /*
   * look carefully to the life cycle of the buffer,
   * this function only record the address and length of the chunk content,
   * if you want to make a copy,
   * use FromMemory instead.
   */
  static std::shared_ptr<Chunk> FromMemoryRef(void *start, size_t size) {
    return std::make_shared<Chunk>(start, false, size);
  }
  // static std::shared_ptr<Chunk> FromFile(std::string file_name) {
  //     std::ifstream in(file_name, std::ios::binary);
  //     auto buf = new unsigned char[in.size];
  //     in.read(buf, in.size);
  //     return std::make_shared<Chunk>(buf);
  // }
  // static std::shared_ptr<Chunk> FromFile(std::string file_name, size_t
  // start_pos, size_t size) {
  //     std::ifstream in(file_name, std::ios::binary);
  //     auto buf = new unsigned char[in.size];
  //     in.seek(start_pos, std::ios::beg);
  //     in.read(buf, in.size);
  //     return std::make_shared<Chunk>(buf);
  // }

private:
  unsigned char *buf_ = nullptr;
  bool needs_to_free_ = false;
  int length_;
};
} // namespace delta