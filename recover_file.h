#include "storage/file_meta.h"
#include "storage/storage.h"
#include <filesystem>
namespace Delta {
class RecoverFile {
public:
  void RecoverSingleFile();
  void RecoverAllFiles();
};
}