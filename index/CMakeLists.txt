add_library(index STATIC super_feature_index.cpp)
target_link_libraries(index glog gflags)