#include <memory>
#include <string>
#include <cstring>
#include <iostream>
#include <fstream>
namespace delta {
class Chunk {
public:
    ~Chunk() {if (buf_ != nullptr) delete [] buf_;}
    static std::shared_ptr<Chunk> FromMemory(void *start, size_t size) {
        buf = new unsigned char[size];
        memcpy(buf, start, size);
        return std::make_shared<Chunk>(buf);
    }
    static std::shared_ptr<Chunk> FromFile(std::string file_name) {
        std::ifstream in(file_name, std::ios::binary);
        buf = new unsigned char[in.size];
        in.read(buf, in.size);
        return std::make_shared<Chunk>(buf);
    }
    static std::shared_ptr<Chunk> FromFile(std::string file_name, size_t start_pos, size_t size) {
        std::ifstream in(file_name, std::ios::binary);
        buf = new unsigned char[in.size];
        in.seek(start_pos, std::ios::beg);
        in.read(buf, in.size);
        return std::make_shared<Chunk>(buf);
    }
    
private:
    Chunk() = delete;
    Chunk(unsigned char *buf) : buf_(buf) {};
    unsigned char *buf_ = nullptr;
};
} // namespace delta