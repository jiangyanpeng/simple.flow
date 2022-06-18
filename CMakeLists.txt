cmake_minimum_required(VERSION 3.10)
project(dagflow)

set(CMAKE_CXX_STANDARD 11)
file(GLOB_RECURSE src ./*.cpp ./*.h)
include_directories(..)

add_executable(Graph main.cpp)