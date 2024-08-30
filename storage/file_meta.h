#pragma once
#include <fstream>
#include <glog/logging.h>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
namespace Delta {
struct FileMeta {
  std::string file_name;
  uint32_t start_chunk_id;
  uint32_t end_chunk_id;
};

inline std::optional<FileMeta> GetFileMetaByName(const std::string &meta_path,
                                          const std::string &file_name) {
  std::ifstream inFile(meta_path, std::ios::in);
  if (!inFile.is_open()) {
    LOG(FATAL) << "Fail to open " << meta_path;
  }
  FileMeta meta;
  std::string line;
  int foundInteger = -1;
  while (getline(inFile, line)) {
    std::istringstream iss(line);
    if (iss >> meta.file_name >> meta.start_chunk_id >> meta.end_chunk_id) {
      if (meta.file_name == file_name)
        return meta;
    }
  }
  return std::nullopt;
}

class FileMetaWriter {
public:
  FileMetaWriter(const std::string &path) {
    out_ = std::ofstream(path, std::ios::out);
    if (!out_.is_open()) {
      LOG(FATAL) << "Failed to open file " << path;
    }
  }
  FileMetaWriter() = default;
  ~FileMetaWriter() {
    if (out_.is_open())
      out_.close();
  }
  void Init(const std::string &path) {
    out_ = std::ofstream(path, std::ios::out);
    if (!out_.is_open()) {
      LOG(FATAL) << "Failed to open file " << path;
    }
  }
  void Write(const FileMeta &meta) {
    out_ << meta.file_name << " " << meta.start_chunk_id << " "
         << meta.end_chunk_id << std::endl;
  }

private:
  std::ofstream out_;
};

class FileMetaReader {
public:
  FileMetaReader(const std::string &path) {
    in_ = std::ifstream(path, std::ios::in);
    if (!in_.is_open()) {
      LOG(FATAL) << "Failed to open file " << path;
    }
  }
  FileMetaReader() = default;
  ~FileMetaReader() {
    if (in_.is_open())
      in_.close();
  }
  void Init(const std::string &path) {
    in_ = std::ifstream(path, std::ios::in);
    if (!in_.is_open()) {
      LOG(FATAL) << "Failed to open file " << path;
    }
  }
  std::optional<FileMeta> Next() {
    FileMeta meta;
    std::string line;
    int foundInteger = -1;
    if (!getline(in_, line))
      return std::nullopt;
    std::istringstream iss(line);
    iss >> meta.file_name >> meta.start_chunk_id >> meta.end_chunk_id;
    return meta;
  }

private:
  std::ifstream in_;
};
} // namespace Delta