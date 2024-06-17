#pragma once
#include "3party/xdelta3/xdelta3.h"
#include "chunk/chunk.h"
#include "encoder/encoder.h"
#include <iostream>
namespace Delta {
class XDelta : public Encoder {
public:
  std::shared_ptr<Chunk> encode(std::shared_ptr<Chunk> base_chunk,
                                std::shared_ptr<Chunk> new_chunk) override {
    uint64_t delta_chunk_size = 0;
    if (0 != xd3_encode_memory(new_chunk->buf(), new_chunk->len(),
                               base_chunk->buf(), base_chunk->len(),
                               chunk_buffer_, &delta_chunk_size, 10000, 0)) {
      std::cerr << "some wrong happens in XDelta encode" << std::endl;
      return nullptr;
    }
    return Chunk::FromMemoryRef(chunk_buffer_, delta_chunk_size,
                                new_chunk.id());
  }
  std::shared_ptr<Chunk> decode(std::shared_ptr<Chunk> base,
                                std::shared_ptr<Chunk> delta) override {
    uint64_t decode_chunk_size = 0;
    if (0 != xd3_decode_memory(delta->buf(), delta->len(), base->buf(), base->len(),
                               chunk_buffer_, &decode_chunk_size, 10000, 0)) {
      std::cerr << "some wrong happens in XDelta decode" << std::endl;
      return nullptr;
    }
    return Chunk::FromMemoryRef(chunk_buffer_, delta_chunk_size, delta.id());
  }

private:
  uint8_t chunk_buffer_[10000];
};
} // namespace Delta