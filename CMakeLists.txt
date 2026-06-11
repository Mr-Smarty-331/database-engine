cmake_minimum_required(VERSION 3.15)

project(kvstore VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG    release-1.12.1
)

FetchContent_Declare(
  crc32c
  GIT_REPOSITORY https://github.com/google/crc32c.git
  GIT_TAG        1.1.2
)

FetchContent_MakeAvailable(googletest crc32c)

enable_testing()

add_library(kvstore_lib src/slice.cpp src/status.cpp src/log_writer.cpp src/crc32c.cpp)

target_link_libraries(kvstore_lib PUBLIC crc32c)


target_include_directories(kvstore_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(kvstore_main src/main.cpp)	

target_link_libraries(kvstore_main PRIVATE kvstore_lib)

add_subdirectory(tests)
