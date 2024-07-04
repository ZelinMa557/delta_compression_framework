#include "storage/file_meta.h"
#include "storage/storage.h"
#include <filesystem>
namespace Delta {
class RecoverFile {
public:
  void RecoverSingleFile();
  void RecoverAllFiles();
private:
  void RecoverSingle(const FileMeta &meta, const std::string &path);
};
}