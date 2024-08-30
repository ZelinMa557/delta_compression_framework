#include "storage/file_meta.h"
#include "storage/storage.h"
#include <filesystem>
namespace Delta {
class RecoverFile {
public:
  RecoverFile();
  void RecoverSingleFile();
  void RecoverAllFiles();
private:
  void RecoverSingle(const FileMeta &meta, const std::string &path);
  std::unique_ptr<Storage> storage_;
};
}