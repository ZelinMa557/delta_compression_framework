add_library(index STATIC super_feature_index.cpp naive_multi_index_hamming.cpp
                            best_fit_index.cpp)
target_link_libraries(index glog gflags feature)