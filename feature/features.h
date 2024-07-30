#pragma once
#include <memory>
#include <variant>
#include <vector>
namespace Delta {
constexpr int default_finesse_sf_cnt = 3;
// every super feature is grouped with 4 sub-features by default
constexpr int default_finesse_sf_subf = 4;

constexpr int default_odess_sf_cnt = 3;
constexpr int default_odess_sf_subf = 4;
constexpr uint64_t default_odess_mask = (1 << 5) - 1;
class Chunk;
using Feature = std::variant<uint64_t,             // for simhash feature
                             std::vector<uint64_t> // for super feature
                             >;

class FeatureCalculator {
public:
  virtual Feature operator()(std::shared_ptr<Chunk> chunk) = 0;
};

class FinesseFeature : FeatureCalculator {
public:
  FinesseFeature(const int sf_cnt = default_finesse_sf_cnt,
                 const int sf_subf = default_finesse_sf_subf)
      : sf_cnt_(sf_cnt_), sf_subf_(sf_subf) {}

private:
  Feature operator()(std::shared_ptr<Chunk> chunk);
  // grouped super features count
  const int sf_cnt_;
  // how much sub feature does a one super feature contain
  const int sf_subf_;
};

class OdessFeature : FeatureCalculator {
public:
  OdessFeature(const int sf_cnt = default_odess_sf_cnt,
               const int sf_subf = default_odess_sf_subf,
               const int mask = default_odess_mask)
      : sf_cnt_(sf_cnt_), sf_subf_(sf_subf), mask_(mask) {}

private:
  Feature operator()(std::shared_ptr<Chunk> chunk);
  // grouped super features count
  const int sf_cnt_;
  // how much sub feature does a one super feature contain
  const int sf_subf_;

  const int mask_;
};

class SimHashFeature: public FeatureCalculator {
public:
  Feature operator()(std::shared_ptr<Chunk> chunk);
};

class CRCSimHashFeature: public FeatureCalculator {
public:
  CRCSimHashFeature(int sub_chunk = 128): sub_chunk_(sub_chunk) {}
  Feature operator()(std::shared_ptr<Chunk> chunk);
private:
  const int sub_chunk_;
};
} // namespace Delta