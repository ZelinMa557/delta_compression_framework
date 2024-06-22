#pragma once
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string>

class mapped_file {
public:
    mapped_file() = default;
    mapped_file(const std::string &file_name);
    mapped_file(mapped_file&& other) {
        if (mapped_addr) munmap(mapped_addr, mapped_len);
        this->mapped_addr = other.mapped_addr;
        this->mapped_len = other.mapped_len;
        this->file_name = other.file_name;
        other.mapped_addr = nullptr;
        other.mapped_len = 0;
        other.file_name = "";
    }
    ~mapped_file() {
        if (mapped_addr) munmap(mapped_addr, mapped_len);
    }
    uint8_t *get_mapped_addr() const { return mapped_addr; }
    uint64_t get_maped_len() const { return mapped_len; }
    const std::string &get_file_name() const { return file_name; }
    bool map_success() const { return mapped_len != 0; }
    mapped_file(const mapped_file&) = delete;
    mapped_file& operator=(const mapped_file&) = delete;
    mapped_file& operator=(mapped_file&& other) {
        if (mapped_addr) munmap(mapped_addr, mapped_len);
        this->mapped_addr = other.mapped_addr;
        this->mapped_len = other.mapped_len;
        this->file_name = other.file_name;
        other.mapped_addr = nullptr;
        other.mapped_len = 0;
        other.file_name = "";
        return *this;
    }
private:
    uint8_t *mapped_addr = nullptr;
    uint64_t mapped_len = 0;
    std::string file_name;
};

inline mapped_file::mapped_file(const std::string &file_name) {
    int fd = open(file_name.c_str(), O_RDONLY);
    if (fd == -1) {
        return;
    }
    uint64_t len = lseek(fd, 0, SEEK_END);
    uint8_t *addr = (uint8_t*)mmap(NULL, len, PROT_READ, MAP_PRIVATE, fd, 0);
    if (addr == MAP_FAILED) {
        close(fd);
        return;
    }
    this->mapped_addr = addr;
    this->mapped_len = len;
    this->file_name = file_name;
    close(fd);
}