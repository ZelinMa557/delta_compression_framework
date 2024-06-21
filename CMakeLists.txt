cmake_minimum_required(VERSION 3.10)
project(delta_compression_framework)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 99)


include_directories(${CMAKE_SOURCE_DIR})
add_subdirectory(chunk)
add_subdirectory(feature)
add_subdirectory(utils)
add_subdirectory(index)
add_subdirectory(storage)
add_subdirectory(3party/xdelta3)
add_library(delta_compression STATIC delta_compression.cpp)
add_executable(delta main.cpp)
target_link_libraries(delta chunker feature index storage utils delta_compression xdelta3 stdc++fs)

