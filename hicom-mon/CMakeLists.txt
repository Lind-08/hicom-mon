cmake_minimum_required(VERSION 2.8)

project(hicom-mon)

set(CMAKE_C_COMPILER "clang")
set(CMAKE_C_FLAGS "-Wall")

set(CMAKE_C_COMPILER "clang++")
set(CMAKE_C_FLAGS "-Wall")

#add_library(hicom-mon SHARED main.c EBCDICConverter.c)
add_executable(test_conv EBCDICConverter.c main.cpp)
