add_executable(hamming_distance hamming_distance.cpp)
target_link_libraries(hamming_distance chunker feature index storage utils xdelta3 glog gflags)

add_executable(modify_chunk modify_chunk.cpp)
target_link_libraries(modify_chunk feature utils xdelta3 glog gflags)