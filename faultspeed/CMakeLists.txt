cmake_minimum_required(VERSION 3.21.0)
project(FaultSpeed)

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there.\n")
endif()

set(BUILD_GMOCK OFF CACHE BOOL "-" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "-" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "-" FORCE)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "-" FORCE)

add_subdirectory(vendor/benchmark)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS FALSE)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

add_executable(faultspeed src/faultspeed.cpp)
target_link_libraries(faultspeed PRIVATE benchmark::benchmark)
target_compile_options(faultspeed PRIVATE -Wall -Wextra)
target_compile_options(faultspeed PRIVATE -march=skylake -mtune=skylake)
