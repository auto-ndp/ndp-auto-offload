cmake_minimum_required(VERSION 3.21.0)
project(FaultSpeed)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS FALSE)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

add_executable(faultspeed src/faultspeed.cpp)
target_link_libraries(faultspeed PRIVATE ${CONAN_LIBS})
target_compile_options(faultspeed PRIVATE -Wall -Wextra)
