#include "3party/cpptoml.h"
#include "config.h"
#include "delta_compression.h"
#include <filesystem>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <iostream>
#include <memory>
#include <string>
DEFINE_string(config, "finesse.toml", "path to config file");
using namespace Delta;
int main(int argc, char *argv[]) {
  google::InitGoogleLogging(argv[0]);
  FLAGS_stderrthreshold = google::INFO;
  Config::Instance().Init(FLAGS_config);
  std::string config_file;
  auto task = Config::Instance().get()->get_as<std::string>("task");
  if (*task == "compression") {
    std::unique_ptr<Delta::DeltaCompression> compression;
    auto task_data_dir =
        Config::Instance().get()->get_as<std::string>("task_data_dir");
    for (const auto &entry :
         std::filesystem::recursive_directory_iterator(*task_data_dir)) {
      if (entry.is_regular_file()) {
        LOG(INFO) << "start processing file "
                  << entry.path().relative_path().string();
        compression->AddFile(entry.path().relative_path().string());
      }
    }
  }
  return 0;
}