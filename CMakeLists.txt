cmake_minimum_required(VERSION 3.10)
project(delta_compression_framework)
set(CMAKE_CXX_STANDARD 17)


include_directories(${CMAKE_SOURCE_DIR})
add_subdirectory(chunk)
# add_subdirectory(encoder)
add_subdirectory(feature)
add_subdirectory(utils)
add_subdirectory(index)
add_subdirectory(storage)