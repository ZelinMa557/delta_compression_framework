add_library(feature STATIC features.cpp)
target_link_libraries(feature crc32c)