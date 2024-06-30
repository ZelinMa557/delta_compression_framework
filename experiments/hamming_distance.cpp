/*
 * test the hamming distance of CRC-SIMHASH feature between similar chunks
 */
#include "chunk/fast_cdc.h"
#include "dedup/dedup.h"
#include "encoder/xdelta.h"
#include "index/super_feature_index.h"
#include "storage/storage.h"
#include <filesystem>
#include <iostream>
#include <map>
#include <string>
using namespace Delta;
DEFINE_string(feature, "finesse", "choice: finesse, odess");
int hamming_distance(uint64_t a, uint64_t b) {
  uint64_t c = a ^ b;
  int result = 0;
  for (int i = 0; i < 64; i++) {
    result += (c & 1);
    c = (c >> 1);
  }
  return result;
}

int main(int argc, char *argv[]) {
  std::string task_data_dir = "./test_data/glib";
  std::string dedup_path = "./test_data/default.dedup";
  std::string data_path = "./test_data/default.data";
  std::string meta_path = "./test_data/default.meta";
  google::InitGoogleLogging(argv[0]);
  std::map<int, int> hamming_distance_count;
  auto chunker = FastCDC(4096, 4096 * 4, 8192);
  auto dedup = Dedup(dedup_path);
  SuperFeatureIndex::SuperFeatureType feature_type;
  if (FLAGS_feature == "finesse")
    feature_type == SuperFeatureIndex::SuperFeatureType::Finesse;
  else if (FLAGS_feature == "odess")
    feature_type == SuperFeatureIndex::SuperFeatureType::Odess;
  auto index = SuperFeatureIndex(feature_type);
  auto storage =
      Storage(data_path, meta_path, std::make_unique<XDelta>(), true, 200);
  for (const auto &entry :
       std::filesystem::recursive_directory_iterator(task_data_dir)) {
    if (entry.is_regular_file()) {
      chunker.ReinitWithFile(entry.path().relative_path().string());
      while (true) {
        auto chunk = chunker.GetNextChunk();
        if (nullptr == chunk)
          break;
        uint32_t dedup_base_id = dedup.ProcessChunk(chunk);
        // duplicate chunk
        if (dedup_base_id != chunk->id()) {
          storage.WriteDuplicateChunk(chunk, dedup_base_id);
          continue;
        }

        auto base_chunk_id = index.GetBaseChunkID(chunk, true);
        if (base_chunk_id.value() == chunk->id()) {
          storage.WriteBaseChunk(chunk);
        } else {
          storage.WriteDeltaChunk(chunk, base_chunk_id.value());
          auto base_chunk_feature =
              CRCSimHashFeature(storage.GetChunkContent(base_chunk_id.value()));
          auto chunk_feature = CRCSimHashFeature(chunk);
          hamming_distance_count[hamming_distance(base_chunk_feature,
                                                  chunk_feature)]++;
        }
      }
    }
  }

  for (const auto &[hamming_distance, count] : hamming_distance_count) {
    std::cout << "hamming_distance " << hamming_distance << " count " << count
              << std::endl;
  }
  return 0;
}