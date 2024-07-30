#pragma once
#include <memory>
#include "chunk/chunk.h"
namespace Delta {
/*
* filter out the false postive similar chunks.
*/
class FilterStrategy {
public:
  /*
  * base: the detected base chunk
  * delta: the detected delta chunk
  * return: filter out or not.
  */
  virtual bool operator()(std::shared_ptr<Chunk> base, std::shared_ptr<Chunk> delta) = 0;
};

class NotFilterStrategy: public FilterStrategy {
public:
  bool operator()(std::shared_ptr<Chunk> base, std::shared_ptr<Chunk> delta) override {
    return false;
  }
};

class FilterByDeltaEncoder: public FilterStrategy {
public:
  /*
  * NOTE(mxk): this class is special.
  * arg `delta` is the delta encoded chunk
  */
  bool operator()(std::shared_ptr<Chunk> base, std::shared_ptr<Chunk> delta) override {
    return (double)delta->len() / (double)base->len() > 0.25;
  }
};
} // namespace Delta