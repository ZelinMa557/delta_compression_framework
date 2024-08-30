#include "3party/cpptoml.h"
#include "config.h"
#include "delta_compression.h"
#include "pipeline_delta_compression.h"
#include "recover_file.h"
#include <filesystem>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <iostream>
#include <memory>
#include <string>
DEFINE_string(config, "odess.toml", "path to config file");
using namespace Delta;
int main(int argc, char *argv[]) {
  FLAGS_stderrthreshold = google::INFO;
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  LOG(INFO) << "using config file " << FLAGS_config;
  Config::Instance().Init(FLAGS_config);
  auto task = Config::Instance().get()->get_as<std::string>("task");
  if (*task == "compression") {
    Delta::DeltaCompression *compression;
    auto pipeline = Config::Instance()
                        .get()
                        ->get_as<bool>("pipeline")
                        .value_or<bool>(false);
    if (pipeline) {
      compression = new Delta::PipelineDeltaCompression();
    } else {
      compression = new Delta::DeltaCompression();
    }
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
    delete compression;
  }

  else if (*task == "recover_single") {
    RecoverFile recover_file;
    recover_file.RecoverSingleFile();
  }

  else if (*task == "recover_all") {
    RecoverFile recover_file;
    recover_file.RecoverAllFiles();
  }
  return 0;
}