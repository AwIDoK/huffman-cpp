#ifndef HUFFMAN_LIBRARY_H
#define HUFFMAN_LIBRARY_H

#include <istream>

namespace huffman {
    void encode(std::istream &in, std::ostream &out);
    void decode(std::istream &in, std::ostream &out);
}

#endif