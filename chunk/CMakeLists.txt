add_library(chunker STATIC rabin_cdc.cpp fast_cdc.cpp crc_cdc.cpp)
target_link_libraries(chunker glog gflags)