cmake_minimum_required(VERSION 3.27)
project(systrace)

set(CMAKE_CXX_STANDARD 20)

add_executable(systrace main.cpp syscalls.cpp)
