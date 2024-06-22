#pragma once
#include <iostream>
#include <memory>
namespace Delta {
class Chunk;
class Encoder {
public:
  virtual std::shared_ptr<Chunk> encode(std::shared_ptr<Chunk> input,
                                        std::shared_ptr<Chunk> source) = 0;
  virtual std::shared_ptr<Chunk> decode(std::shared_ptr<Chunk> base,
                                        std::shared_ptr<Chunk> delta) = 0;
};
} // namespace Delta
