#include "feature/features.h"
#include "index/index.h"
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>
namespace Delta {
namespace {
inline std::vector<uint64_t> getFinesseFeature(std::shared_ptr<Chunk> chunk,
                                               const int sf_cnt) {
  return FinesseFeature(chunk, sf_cnt);
}
inline std::vector<uint64_t> getOdessFeature(std::shared_ptr<Chunk> chunk,
                                             const int sf_cnt) {
  return OdessFeature(chunk, sf_cnt);
}
} // namespace

using chunk_id = uint32_t;
class Chunk;
class SuperFeatureIndex : public Index {
public:
  enum class SuperFeatureType { Finesse, Odess };
  SuperFeatureIndex(SuperFeatureType feature_type, int super_feature_count = 3)
      : feature_type_(feature_type), super_feature_count_(super_feature_count) {
    for (int i = 0; i < super_feature_count_; i++) {
      index_.push_back({});
    }
    switch (feature_type_) {
    case SuperFeatureType::Finesse:
      get_feature_ = getFinesseFeature;
      break;
    case SuperFeatureType::Odess:
      get_feature_ = getOdessFeature;
      break;
    }
  }
  std::optional<chunk_id> GetBaseChunkID(std::shared_ptr<Chunk> chunk,
                                         bool add_new_base_chunk);
  bool RecoverFromFile(const std::string &path);
  bool DumpToFile(const std::string &path);

private:
  std::vector<std::unordered_map<uint64_t, chunk_id>> index_;
  SuperFeatureType feature_type_;
  int super_feature_count_;
  std::function<std::vector<uint64_t>(std::shared_ptr<Chunk>, const int)>
      get_feature_;
};
} // namespace Delta