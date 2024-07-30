#include "feature/features.h"
#include "index/index.h"
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>
namespace Delta {

using chunk_id = uint32_t;
class Chunk;
class SuperFeatureIndex : public Index {
public:
  SuperFeatureIndex(int super_feature_count = 3)
      : super_feature_count_(super_feature_count) {
    for (int i = 0; i < super_feature_count_; i++) {
      index_.push_back({});
    }
  }
  std::optional<chunk_id> GetBaseChunkID(const Feature &feat);
  void AddFeature(const Feature &feat, chunk_id id);
  bool RecoverFromFile(const std::string &path);
  bool DumpToFile(const std::string &path);

private:
  std::vector<std::unordered_map<uint64_t, chunk_id>> index_;
  int super_feature_count_;
  std::function<std::vector<uint64_t>(std::shared_ptr<Chunk>, const int)>
      get_feature_;
};
} // namespace Delta