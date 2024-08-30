#include "recover_file.h"
#include "chunk/chunk.h"
#include "encoder/xdelta.h"
#include "config.h"
namespace Delta {
void RecoverFile::RecoverSingle(const FileMeta &meta, const std::string &path) {
  std::ofstream out(path, std::ios::out);
  for (uint32_t i = meta.start_chunk_id; i <= meta.end_chunk_id; i++) {
    auto chunk = storage_->GetChunkContent(i);
    out.write((char*)chunk->buf(), chunk->len());
  }
  out.close();
  LOG(INFO) << "Recover file " << meta.file_name << " finish.";
}

void RecoverFile::RecoverSingleFile() {
  auto config = Config::Instance().get();
  auto file_to_recover = *config->get_as<std::string>("file_to_recover");
  auto output_dir = *config->get_as<std::string>("output_dir");
  auto file_meta_path = *config->get_as<std::string>("file_meta_path");
  std::string output_file_path = output_dir + file_to_recover;

  auto meta = GetFileMetaByName(file_meta_path, file_to_recover);
  if (!meta.has_value())
    LOG(FATAL) << "Fail to recover file " << file_to_recover;
  RecoverSingle(meta.value(), output_file_path);
}

void RecoverFile::RecoverAllFiles() {
  auto config = Config::Instance().get();
  auto output_dir = *config->get_as<std::string>("output_dir");
  auto file_meta_path = *config->get_as<std::string>("file_meta_path");

  FileMetaReader meta_reader(file_meta_path);
  while (auto meta = meta_reader.Next()) {
    if (!meta.has_value())
      break;
    std::string output_file_path = output_dir + meta.value().file_name;
    RecoverSingle(meta.value(), output_file_path);
  }
}

RecoverFile::RecoverFile() {
  auto config = Config::Instance().get();
  auto chunk_data_path = *config->get_as<std::string>("chunk_data_path");
  auto chunk_meta_path = *config->get_as<std::string>("chunk_meta_path");
  auto storage = config->get_table("storage");
  auto encoder_name = *storage->get_as<std::string>("encoder");
  auto cache_size = *storage->get_as<int64_t>("cache_size");
  std::unique_ptr<Encoder> encoder;
  if (encoder_name == "xdelta") {
    encoder = std::make_unique<XDelta>();
  } else {
    LOG(FATAL) << "Unknown encoder type " << encoder_name;
  }
  this->storage_ = std::make_unique<Storage>(
      chunk_data_path, chunk_meta_path, std::move(encoder), false, cache_size);
}
} // namespace Delta