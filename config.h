#pragma once
#include <3party/cpptoml.h>
#include <string>
namespace Delta {
class Config {
private:
  Config() {}
  std::shared_ptr<cpptoml::table> conf_;
public:
  void Init(std::string &config_file) {
    conf_ = cpptoml::parse_file(config_file);
  }
  std::shared_ptr<cpptoml::table> get() {
    return conf_;
  }
  static Config& Instance() {
    static Config config_;
    return config_;
  }
};
}