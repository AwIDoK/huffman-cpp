#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <sstream>
#include <utility>
#include "gtest/gtest.h"

#include "../huffman_archiver/huffman.h"

namespace {
    std::string compress_decompress(std::string &str) {
        std::string result;
        std::stringstream encoded, decoded;
        std::stringstream input(str);
        huffman::encode(input, encoded);
        huffman::decode(encoded, decoded);
        return decoded.str();
    }

    std::string random_string(size_t len) {
        std::string result;
        for (size_t i = 0; i < len; i++) {
            result += char(std::rand() % 26 + 'a');
        }
        return result;
    }
}

TEST(correctness, empty)
{
    std::string test;
    EXPECT_EQ(test, compress_decompress(test));
}

TEST(correctness, same_symbol)
{
    for (size_t i = 1; i <= 512; i++) {
        std::string test = "a";
        EXPECT_EQ(test, compress_decompress(test));
    }
}

TEST(correctness, corrupted_hash)
{
    std::string test = random_string(1024*1024);
    std::stringstream input(test), encoded, result;
    huffman::encode(input, encoded);
    auto corrupted = encoded.str();
    corrupted[2]++;
    std::stringstream corrupted_stream(corrupted);
    EXPECT_ANY_THROW(huffman::decode(corrupted_stream, result));
}

TEST(correctness, corrupted_file)
{
    std::string test = random_string(1024*1024);
    std::stringstream input(test), encoded, result;
    huffman::encode(input, encoded);
    auto corrupted = encoded.str();
    corrupted[1000]++;
    std::stringstream corrupted_stream(corrupted);
    EXPECT_ANY_THROW(huffman::decode(corrupted_stream, result));
}

TEST(correctness, smaller_than_hash)
{
    std::string test = random_string(7);
    std::stringstream input(test), encoded, result;
    EXPECT_ANY_THROW(huffman::decode(input, result));
}


TEST(correctness, random_string_with_different_length)
{
    for (size_t i = 1; i <= 1025; i++) {
        std::string test = random_string(i);
        EXPECT_EQ(test, compress_decompress(test));
    }
}

TEST(correctness, one_symbol_copression)
{
    std::string test(1024 * 1024, 'a');
    std::stringstream input(test), encoded;
    huffman::encode(input, encoded);
    EXPECT_EQ(1024 * 1024. / encoded.str().size() > 7.8, true);
}


TEST(correctness, random_strings)
{
    for (size_t i = 1; i <= 50; i++) {
        std::string test = random_string(1024*1024);
        EXPECT_EQ(test, compress_decompress(test));
    }
}

TEST(correctness, random_big_strings)
{
    for (int i = -5; i <= 5; i++) {
        std::string test = random_string(static_cast<size_t>(1024 * 1024 * 7 + i));
        EXPECT_EQ(test, compress_decompress(test));
    }
}




