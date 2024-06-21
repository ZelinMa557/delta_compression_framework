#include "3party/cpptoml.h"
#include "delta_compression.h"
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
int main(int argc, char *argv[]) {
  std::string config_file;
  if (argc == 2) {
    config_file = std::string(argv[1]);
  } else {
    config_file = "config.toml";
  }
  auto config = cpptoml::parse_file("config.toml");
  auto index_path = config->get_as<std::string>("index_path");
  auto data_path = config->get_as<std::string>("data_path");
  auto meta_path = config->get_as<std::string>("meta_path");

  auto task = config->get_as<std::string>("task");
  auto algorithm = config->get_as<std::string>("algorithm");
  if (*task == "compression") {
    std::unique_ptr<Delta::DeltaCompression> compression = nullptr;
    if (*algorithm == "finesse") {
      compression.reset(std::move(Delta::DeltaCompression::MakeFinesse(
          *data_path, *meta_path, *index_path)));
    } else if (algorithm == "odess") {
      compression.reset(std::move(Delta::DeltaCompression::MakeOdess(
          *data_path, *meta_path, *index_path)));
    }
    auto task_data_dir = config->get_as<std::string>("task_data_dir");
    for (const auto &entry :
         std::filesystem::recursive_directory_iterator(*task_data_dir)) {
      if (entry.is_regular_file()) {
        compression->AddFile(entry.path().relative_path().string());
      }
    }
  }
  return 0;
}