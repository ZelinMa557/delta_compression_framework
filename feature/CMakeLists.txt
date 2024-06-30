add_library(feature STATIC finesse_feature.cpp odess_feature.cpp crc_simhash_feature.cpp)
target_link_libraries(feature crc32c)