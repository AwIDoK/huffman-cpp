#include <fstream>
#include <iostream>
#include <cstring>
#include "../huffman_archiver/huffman.h"

int main(int argc, char* argv[]) {
    if (argc != 4 || (!std::strcmp("-c", argv[1]) && !std::strcmp("-d", argv[1]))) {
        std::cout << "Usage:\n -c input_filename output_filename\tfor compression\n -d input_filename output_filename\tfor decompression\n";
        return 0;
    }
    try {
        if (std::strcmp(argv[2], argv[3]) == 0) {
            std::cerr << "input_file and output_file are same\n";
            return 0;
        }
        std::ifstream in(argv[2], std::ios_base::binary);
        std::ofstream out(argv[3], std::ios_base::binary);
        if (!in.is_open()) {
            std::cerr << "input_file cannot be opened\n";
            return 0;
        }
        if (!out.is_open()) {
            std::cerr << "output_file cannot be opened\n";
            return 0;
        }
        if (std::strcmp("-c", argv[1]) == 0) {
            huffman::encode1(in, out);
        } else {
            huffman::decode(in, out);
        }
    } catch(std::exception const &ex) {
        std::cerr << ex.what();
    }
}

