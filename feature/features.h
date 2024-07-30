#pragma once
#include <memory>
#include <vector>
#include <variant>
namespace Delta {
constexpr int default_finesse_sf_cnt = 3;
// every super feature is grouped with 4 sub-features by default
constexpr int default_finesse_sf_subf = 4;

constexpr int default_odess_sf_cnt = 3;
constexpr int default_odess_sf_subf = 4;
constexpr uint64_t default_odess_mask = (1 << 5) - 1;
class Chunk;
using Feature = std::variant<uint64_t, // for simhash feature
                             std::vector<uint64_t> // for super feature
                             >;
/*
 * Calculate Finesse super features.
 * @param chunk the chunk to calculate
 * @param sf_cnt grouped super features count
 * @param sf_subf how much sub feature does a one super feature contain
 * return the calculated Finesse super features
 */
Feature
FinesseFeature(std::shared_ptr<Chunk> chunk,
               const int sf_cnt = default_finesse_sf_cnt,
               const int sf_subf = default_finesse_sf_subf);
/*
 * Calculate Odess super features.
 * @param chunk the chunk to calculate
 * @param sf_cnt grouped super features count
 * @param sf_subf how much sub feature does a one super feature contain
 * return the calculated Odess super features
 */
Feature OdessFeature(std::shared_ptr<Chunk> chunk,
                                   const int sf_cnt = default_finesse_sf_cnt,
                                   const int sf_subf = default_finesse_sf_subf,
                                   uint64_t mask = default_odess_mask);

Feature SimHashFeature(std::shared_ptr<Chunk> chunk,
                           const int sub_chunk = 128);

Feature CRCSimHashFeature(std::shared_ptr<Chunk> chunk,
                           const int sub_chunk = 128);
} // namespace Delta