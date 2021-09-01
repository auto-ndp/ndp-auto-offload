cmake_minimum_required(VERSION 3.12)

project(
  "memspeed"
  VERSION 0.1.0
  LANGUAGES CXX
)

message(STATUS "Started CMake for ${PROJECT_NAME} v${PROJECT_VERSION}...\n")

option(ENABLE_UPMEM_DPU "Enables upmem's DPU benchmark" OFF)

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there.\n")
endif()

add_subdirectory(vendor/benchmark)

set(BUILD_GMOCK OFF CACHE BOOL "-" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "-" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "-" FORCE)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "-" FORCE)

set(exe_sources
  src/memspeed.cpp
  src/cpuonly.cpp
)

set(cpp_defines)

if(ENABLE_UPMEM_DPU)
  list(APPEND exe_sources src/upmemdpu.cpp)
  list(APPEND cpp_defines ENABLE_UPMEM_DPU)
endif()

add_executable(${PROJECT_NAME} ${exe_sources})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
target_compile_definitions(${PROJECT_NAME} PUBLIC ${cpp_defines})
target_link_libraries(${PROJECT_NAME} PUBLIC benchmark::benchmark)
target_compile_options(${PROJECT_NAME} PUBLIC -march=skylake -mtune=skylake)

if(ENABLE_UPMEM_DPU)
  target_include_directories(${PROJECT_NAME} PUBLIC /usr/include/dpu)
  target_link_libraries(${PROJECT_NAME} PUBLIC dpu)
endif()

include(../cmake/CompilerWarnings.cmake)
set_project_warnings(${PROJECT_NAME})

# Provide standard alias
add_executable(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
