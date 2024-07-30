#pragma once
#include "3party/xdelta3/xdelta3.h"
#include "chunk/chunk.h"
#include "encoder/encoder.h"
#include <glog/logging.h>
namespace Delta {
class XDelta : public Encoder {
public:
  std::shared_ptr<Chunk> encode(std::shared_ptr<Chunk> base_chunk,
                                std::shared_ptr<Chunk> new_chunk) override {
    long unsigned int delta_chunk_size = 0;
    int result = xd3_encode_memory(new_chunk->buf(), new_chunk->len(),
                                   base_chunk->buf(), base_chunk->len(),
                                   chunk_buffer_, &delta_chunk_size, 32768, 0);
    if (0 != result) {
      LOG(FATAL) << "some wrong happens in XDelta encode, returns " << result << " delta chunk size " << delta_chunk_size;
    }
    LOG(INFO) << "delta chunk size is " << delta_chunk_size;
    return Chunk::FromMemoryRef(chunk_buffer_, delta_chunk_size,
                                new_chunk->id());
  }
  std::shared_ptr<Chunk> decode(std::shared_ptr<Chunk> base,
                                std::shared_ptr<Chunk> delta) override {
    long unsigned int decode_chunk_size = 0;
    if (0 != xd3_decode_memory(delta->buf(), delta->len(), base->buf(),
                               base->len(), chunk_buffer_, &decode_chunk_size,
                               32768, 0)) {
      LOG(FATAL) << "some wrong happens in XDelta decode, returns ";
      return nullptr;
    }
    LOG(INFO) << "decoded chunk size is " << decode_chunk_size;
    return Chunk::FromMemoryRef(chunk_buffer_, decode_chunk_size, delta->id());
  }

private:
  uint8_t chunk_buffer_[32768];
};
} // namespace Delta