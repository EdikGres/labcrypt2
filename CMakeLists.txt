cmake_minimum_required(VERSION 3.24)
project(lab2_cript_cpp)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(lab2_cript_cpp main.cpp cast_256.cpp cast_256.h)
