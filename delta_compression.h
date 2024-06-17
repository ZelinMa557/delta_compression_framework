#include "chunk/chunker.h"
#include "chunk/encoder.h"
#include "index/index.h"
#include "storage/storage.h"
#include <memory>
#include <string>
namespace Delta {
class DeltaCompression {
public:
  DeltaCompression(std::string out_data_path, std::string out_meta_path,
                   std::string index_path, std::unique_ptr<Chunker> chunker,
                   std::unique_ptr<Encoder> encoder,
                   std::unique_ptr<Index> index)
      : out_data_path_(out_data_path), out_meta_path_(out_meta_path),
        index_path_(index_path), chunker_(std::move(chunker)),
        index_(std::move(index)),
        storage_(out_data_path, out_meta_path, std::move(encoder), true) {}

  static std::unique_ptr<DeltaCompression>
  MakeFinesse(std::string out_data_path, std::string out_meta_path,
              std::string index_path);
  static std::unique_ptr<DeltaCompression> MakeOdess(std::string out_data_path,
                                                     std::string out_meta_path,
                                                     std::string index_path);
  void AddFile(const std::string &file_name);

private:
  std::string out_data_path_;
  std::string out_meta_path_;
  std::string index_path_;

  std::unique_ptr<Chunker> chunker_;
  std::unique_ptr<Index> index_;

  Storage storage_;
};
} // namespace Delta