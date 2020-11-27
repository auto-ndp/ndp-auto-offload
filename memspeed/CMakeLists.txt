cmake_minimum_required(VERSION 3.12)

project(
  "memspeed"
  VERSION 0.1.0
  LANGUAGES CXX
)

message(STATUS "Started CMake for ${PROJECT_NAME} v${PROJECT_VERSION}...\n")

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there.\n")
endif()

set(exe_sources
  src/memspeed.cpp
)

add_executable(${PROJECT_NAME} ${exe_sources})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

include(../cmake/CompilerWarnings.cmake)
set_project_warnings(${PROJECT_NAME})

# Provide standard alias
add_executable(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
