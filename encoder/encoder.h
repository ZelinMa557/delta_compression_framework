#include <iostream>
#include <memory>
namespace Delta {
class Chunk;
class Encoder {
public:
  virtual std::shared_ptr<Chunk> encode(std::shared_ptr<Chunk> src,
                                        std::shared_ptr<Chunk> tgt);
  virtual std::shared_ptr<Chunk> decode(std::shared_ptr<Chunk> src,
                                        std::shared_ptr<Chunk> delta);
};
} // namespace Delta
