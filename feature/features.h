#pragma once
#include <memory>
#include <vector>
namespace delta {
constexpr int default_finesse_sf_cnt = 3;
// every super feature is grouped with 4 sub-features by default
constexpr int default_finesse_sf_subf = 4;

constexpr int default_odess_sf_cnt = 3;
constexpr int default_odess_sf_subf = 4;
constexpr uint64_t default_odess_mask = (1 << 5) - 1;
class Chunk;
/*
 * Calculate Finesse super features.
 * @param chunk the chunk to calculate
 * @param sf_cnt grouped super features count
 * @param sf_subf how much sub feature does a one super feature contain
 * return the calculated Finesse super features
 */
std::vector<uint64_t>
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
std::vector<uint64_t> OdessFeature(std::shared_ptr<Chunk> chunk,
                                   const int sf_cnt = default_finesse_sf_cnt,
                                   const int sf_subf = default_finesse_sf_subf,
                                   uint64_t mask = default_odess_mask);
} // namespace delta