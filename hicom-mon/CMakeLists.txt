cmake_minimum_required(VERSION 2.8)

project(hicom-mon)

set(CMAKE_C_COMPILER "clang")
set(CMAKE_C_FLAGS "-fpic -Wall")

set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_CXX_FLAGS "-Wall")

find_library(PCRE_LIBRARY pcre)

find_library(PTHREAD_LIBRARY pthread)

add_library(hicom-mon SHARED main.c EBCDICConverter.c)
#add_executable(hicom-mon main.c EBCDICConverter.c)
#add_executable(test_conv EBCDICConverter.c main.cpp)
target_link_libraries(hicom-mon ${PTHREAD_LIBRARY} ${PCRE_LIBRARY})
