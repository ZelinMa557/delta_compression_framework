#include "encoder/encoder.h"
namespace delta {
class XDelta : public Encoder {
public:
  std::shared_ptr<Chunk> encode(std::shared_ptr<Chunk> src,
                                std::shared_ptr<Chunk> tgt) override;
  std::shared_ptr<Chunk> decode(std::shared_ptr<Chunk> src,
                                std::shared_ptr<Chunk> delta) override;
};
} // namespace delta