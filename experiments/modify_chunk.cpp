#include "chunk/chunk.h"
#include "feature/features.h"
#include <cstdint>
#include <functional>
#include <map>
#include <random>
#include <vector>
#include <cstdio>
#include <string>
constexpr int SEED = 2002;
constexpr int CHUNK_SIZE = 4096;
constexpr int num_iter = 1024;

int hamming_distance(uint64_t a, uint64_t b) {
  uint64_t c = a ^ b;
  int result = 0;
  for (int i = 0; i < 64; i++) {
    result += (c & 1);
    c = (c >> 1);
  }
  return result;
}

std::vector<float> test_modify_some_bytes(
    int chunk_size, int bytes_modify, int num_iter,
    std::vector<std::function<uint64_t(std::shared_ptr<Delta::Chunk>)>> features) {
  std::vector<float> total_hammings(features.size(), 0);
  std::mt19937 generator(SEED);
  std::uniform_int_distribution<uint8_t> distribution(0, 255);
  std::uniform_int_distribution<int> pos_distribution(0, chunk_size-bytes_modify);
  uint8_t *origin = new uint8_t[chunk_size];
  uint8_t *modified = new uint8_t[chunk_size];
  for (int i = 0; i < num_iter; i++) {
    for (int j = 0; j < chunk_size; j++) {
      modified[j] = origin[j] = distribution(generator);
    }
    int modify_pos = pos_distribution(generator);
    for (int j = modify_pos; j < modify_pos + bytes_modify; j++) {
      modified[j] = 255 - origin[j];
    }
    auto origin_chk = Delta::Chunk::FromMemoryRef(origin, chunk_size, 0);
    auto modified_chk = Delta::Chunk::FromMemoryRef(modified, chunk_size, 1);
    for (int j = 0; j < total_hammings.size(); j++) {
      total_hammings[j] +=
        hamming_distance(features[j](origin_chk), features[j](modified_chk));
    }
  }
  delete[] origin;
  delete[] modified;
  for (auto &distance: total_hammings)
    distance = (float)distance / (float)num_iter;
  return total_hammings;
}

std::vector<float> test_insert_some_bytes(
    int chunk_size, int bytes_insert, int num_iter,
    std::vector<std::function<uint64_t(std::shared_ptr<Delta::Chunk>)>> features) {
  std::vector<float> total_hammings(features.size(), 0);
  std::mt19937 generator(SEED);
  std::uniform_int_distribution<uint8_t> distribution(0, 255);
  std::uniform_int_distribution<int> pos_distribution(0, chunk_size);
  uint8_t *origin = new uint8_t[chunk_size];
  uint8_t *modified = new uint8_t[chunk_size + bytes_insert];
  for (int i = 0; i < num_iter; i++) {
    for (int j = 0; j < chunk_size; j++) {
      origin[j] = distribution(generator);
    }
    int insert_pos = pos_distribution(generator);
    int pos = 0;
    for (; pos < insert_pos; pos++) {
      modified[pos] = origin[pos];
    }
    for (; pos < insert_pos + bytes_insert; pos++) {
      modified[pos] = distribution(generator);
    }
    for (; pos < chunk_size + bytes_insert; pos++) {
      modified[pos] = origin[pos-bytes_insert];
    }
    auto origin_chk = Delta::Chunk::FromMemoryRef(origin, chunk_size, 0);
    auto modified_chk = Delta::Chunk::FromMemoryRef(modified, chunk_size+bytes_insert, 1);
    for (int j = 0; j < total_hammings.size(); j++) {
      total_hammings[j] +=
        hamming_distance(features[j](origin_chk), features[j](modified_chk));
    }
  }
  delete[] origin;
  delete[] modified;
  for (auto &distance: total_hammings)
    distance = (float)distance / (float)num_iter;
  return total_hammings;
}

int main() {
  auto simhash = [](std::shared_ptr<Delta::Chunk> chunk) -> uint64_t {
    return Delta::CRCSimHashFeature(chunk, 12);
  };
  std::vector<std::string> hash_names = {"simhash"};
  std::vector<std::function<uint64_t(std::shared_ptr<Delta::Chunk>)>> features = {simhash};
  std::vector<int> test_bytes = {16, 32, 64, 128, 256, 512, 1024, 2048, 4096};
  printf("Testing modify bytes:\n");
  printf("%10s", "bytes");
  for (auto &hash_name: hash_names) {
    printf("%10s", hash_name.c_str());
  }
  putchar('\n');
  for (const auto test_byte: test_bytes) {
    printf("%10d", test_byte);
    auto result = test_modify_some_bytes(CHUNK_SIZE, test_byte, num_iter, features);
    for (auto r : result) {
      printf("%10.3f", r);
    }
    putchar('\n');
  }
  putchar('\n');

  printf("Testing insert bytes:\n");
  printf("%10s", "bytes");
  for (auto &hash_name: hash_names) {
    printf("%10s", hash_name.c_str());
  }
  putchar('\n');
  for (const auto test_byte: test_bytes) {
    printf("%10d", test_byte);
    auto result = test_insert_some_bytes(CHUNK_SIZE, test_byte, num_iter, features);
    for (auto r : result) {
      printf("%10.3f", r);
    }
    putchar('\n');
  }
  return 0;
}