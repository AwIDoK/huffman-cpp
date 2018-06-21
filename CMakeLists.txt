cmake_minimum_required(VERSION 2.8)

project(huffman)

include_directories(${HUFFMAN_SOURCE_DIR})

add_library(huffman_library STATIC huffman_archiver/huffman.cpp huffman_archiver/huffman.h)

add_executable(huffman_testing
        huffman_testing/huffman_testing.cpp
        huffman_testing/gtest/gtest-all.cc
        huffman_testing/gtest/gtest.h
        huffman_testing/gtest/gtest_main.cc)

add_executable(console_util huffman_util/console_util.cpp)

if (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++17 -pedantic")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address,undefined -D_GLIBCXX_DEBUG")
endif ()

target_link_libraries(huffman_testing huffman_library -lpthread)
target_link_libraries(console_util huffman_library -lpthread)