cmake_minimum_required(VERSION 3.15)

project(kvstore VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

add_library(kvstore_lib src/slice.cpp src/status.cpp)

target_include_directories(kvstore_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(kvstore_main src/main.cpp)	

target_link_libraries(kvstore_main PRIVATE kvstore_lib)
