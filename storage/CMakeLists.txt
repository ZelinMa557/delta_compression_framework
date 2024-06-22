add_library(storage STATIC cache.cpp storage.cpp)
target_link_libraries(storage glog gflags)