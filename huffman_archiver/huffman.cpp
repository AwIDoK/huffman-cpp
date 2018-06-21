#include "huffman.h"

#include <vector>
#include <set>
#include <stack>

namespace {
    struct simple_node {
        uint16_t left, right, pos;
        uint8_t value;

        simple_node() : left(0), right(0), pos(0), value(0) {}

        simple_node(uint16_t left, uint16_t right, uint16_t pos, uint8_t value)
                : left(left), right(right), pos(pos), value(value) {}
    };

    struct node : simple_node {
        uint64_t number_of_usages;

        node() : simple_node(0, 0, 0, 0), number_of_usages(0) {}

        node(uint16_t left, uint16_t right, uint16_t pos, uint8_t value, uint64_t number_of_usages)
                : simple_node(left, right, pos, value), number_of_usages(number_of_usages) {}

        bool operator<(node const &second) const {
            return number_of_usages < second.number_of_usages;
        }
    };

    void count(std::vector<uint64_t> &result, std::istream &in) {
        result.assign(1u << 8u, 0);
        uint8_t buffer[1024 * 1024];
        while (true) {
            in.read(reinterpret_cast<char *>(buffer), 1024 * 1024);
            auto len = in.gcount();
            if (len == 0) {
                break;
            }
            for (auto i = 0; i < len; i++) {
                ++result[static_cast<uint8_t>(buffer[i])];
            }
        }
        in.clear();
        in.seekg(std::ios::beg);
    }

    void
    dfs(uint16_t pos, uint32_t depth, std::vector<node> &tree, std::vector<std::pair<uint64_t, uint8_t>> &translate,
        uint64_t value, std::vector<char> &tree_structure, std::vector<uint8_t> &words) {
        if (tree[pos].left == 0) {
            words.push_back(tree[pos].value);
            translate[tree[pos].value].first = value;
            translate[tree[pos].value].second = static_cast<uint8_t>(std::max<uint32_t>(depth, 1u));
        } else {
            tree_structure.push_back(1);
            dfs(tree[pos].left, depth + 1, tree, translate, value, tree_structure, words);
            tree_structure.push_back(0);
            tree_structure.push_back(1);
            dfs(tree[pos].right, depth + 1, tree, translate, value | (1ull << depth), tree_structure, words);
            tree_structure.push_back(0);
        }
    }

    void write(std::ostream &out, uint8_t *data, uint16_t size, uint64_t &hash) {
        for (uint16_t i = 0; i < size; i++) {
            hash = hash * 13 + data[i];
        }
        out.write(reinterpret_cast<const char *>(data), size);
    }

    inline bool is_empty(std::istream &in) {
        return in.peek() == std::istream::traits_type::eof();
    }

    void
    build_tree(std::vector<node> &tree, uint64_t &file_size, uint16_t &number_of_words, std::vector<uint64_t> &usages) {
        tree.emplace_back();
        std::multiset<node> nodes;
        for (uint16_t i = 0; i < 1u << 8u; i++) {
            if (usages[i]) {
                number_of_words++;
                file_size += usages[i];
                node tmp(0, 0, static_cast<uint16_t>(tree.size()), static_cast<uint8_t>(i), 1);
                nodes.insert(tmp);
                tree.push_back(tmp);
            }
        }
        while (nodes.size() > 1) {
            auto first = *nodes.begin();
            nodes.erase(nodes.begin());
            auto second = *nodes.begin();
            nodes.erase(nodes.begin());
            node tmp(first.pos, second.pos, static_cast<uint16_t>(tree.size()), 0,
                     first.number_of_usages + second.number_of_usages);
            nodes.insert(tmp);
            tree.push_back(tmp);
        }
    }

    bool check_hash(std::istream &in, uint8_t *buffer) {
        uint64_t real_hash = 0, written_hash = 0;
        in.read(reinterpret_cast<char *>(&written_hash), 8);
        if (in.gcount() < 8) {
            return false;
        };
        while (true) {
            in.read(reinterpret_cast<char *>(buffer), 1024 * 1024);
            auto len = in.gcount();
            if (len == 0) {
                break;
            }
            for (auto i = 0; i < len; i++) {
                real_hash = real_hash * 13 + buffer[i];
            }
        }
        if (real_hash != written_hash) {
            return false;
        }
        in.clear();
        in.seekg(8, std::ios::beg);
        return true;
    }

    void write_tree(std::ostream &out, std::vector<char> &tree_structure, uint64_t &hash) {
        size_t tree_size = tree_structure.size();
        write(out, reinterpret_cast<uint8_t *>(&tree_size), 2, hash);
        uint8_t buf = 0;
        uint8_t left = 8;
        for (char i : tree_structure) {
            if (i)
                buf |= 1u << (8u - left);
            left--;
            if (left == 0) {
                left = 8;
                write(out, &buf, 1, hash);
                buf = 0;
            }
        }
        if (left != 8) {
            write(out, &buf, 1, hash);
        }
    }

    void encode_file(std::istream &in, std::ostream &out, uint64_t &hash, std::vector<std::pair<uint64_t, uint8_t>> &translate) {
        uint8_t buffer[1024 * 1024];
        uint64_t last = 0;
        uint32_t left = 64;
        while (true) {
            in.read(reinterpret_cast<char *>(buffer), 1024 * 1024);
            auto len = in.gcount();
            if (len == 0) {
                break;
            }
            for (auto i = 0; i < len; i++) {
                last |= translate[buffer[i]].first << (64u - left);
                if (left >= translate[buffer[i]].second) {
                    left -= translate[buffer[i]].second;
                    if (left == 0) {
                        write(out, reinterpret_cast<uint8_t *>(&last), 8, hash);
                        left = 64;
                        last = 0;
                    }
                } else {
                    write(out, reinterpret_cast<uint8_t *>(&last), 8, hash);
                    last = translate[buffer[i]].first >> left;
                    left = 64u - translate[buffer[i]].second + left;
                }
            }
        }
        if (left != 64) {
            write(out, reinterpret_cast<uint8_t *>(&last), 8, hash);
        }
    }

    void read_tree(std::istream &in, std::vector<simple_node> & tree, uint8_t *buffer, char* words) {
        uint16_t tree_size = 0;
        in.read(reinterpret_cast<char *>(&tree_size), 2);
        in.read(reinterpret_cast<char *>(buffer), static_cast<uint16_t>(tree_size / 8u + (tree_size % 8u == 0u ? 0u : 1u)));
        int prev = -1;
        tree.resize(2);
        std::stack<uint16_t> current_nodes;
        current_nodes.push(1);
        uint16_t word_count = 0;
        for (uint16_t i = 0; i < tree_size; i++) {
            int bit = (buffer[i / 8] >> (i % 8u)) & 1u;
            if (bit == 1) {
                if (tree[current_nodes.top()].left == 0) {
                    tree[current_nodes.top()].left = static_cast<uint16_t>(tree.size());
                } else {
                    tree[current_nodes.top()].right = static_cast<uint16_t>(tree.size());
                }
                current_nodes.push(static_cast<uint16_t>(tree.size()));
                tree.emplace_back();
            } else if (bit == 0 && prev == 1) {
                tree.back().value = static_cast<uint8_t>(words[word_count++]);
            }
            if (bit == 0) {
                current_nodes.pop();
            }
            prev = bit;
        }
    }

    void decode_file(std::istream &in, std::ostream &out, std::vector<simple_node> &tree, uint8_t * buffer, char* words, uint64_t file_size) {
        in.read(reinterpret_cast<char *>(buffer), 1024 * 1024);
        uint32_t in_position = 0;
        for (uint64_t i = 0; i < file_size; i++) {
            uint16_t pos = 1;
            if (tree[pos].left == 0) {
                out.write(reinterpret_cast<const char *>(&words[0]), 1);
                in_position++;
                if (in_position >= 1024 * 1024 * 8) {
                    in.read(reinterpret_cast<char *>(buffer), 1024 * 1024);
                    in_position = 0;
                }
            } else {
                while (tree[pos].left != 0) {
                    if ((buffer[in_position / 8u] >> (in_position % 8u)) & 1) {
                        pos = tree[pos].right;
                    } else {
                        pos = tree[pos].left;
                    }
                    in_position++;
                    if (in_position >= 1024 * 1024 * 8) {
                        in.read(reinterpret_cast<char *>(buffer), 1024 * 1024);
                        in_position = 0;
                    }
                }
                out.write(reinterpret_cast<const char *>(&tree[pos].value), 1);
            }
        }
    }
}

void huffman::encode(std::istream &in, std::ostream &out) {
    if (is_empty(in)) {
        return;
    }
    uint64_t hash = 0;
    out.write(reinterpret_cast<char *>(&hash), 8);
    uint64_t file_size = 0;
    uint16_t number_of_words = 0;
    std::vector<uint64_t> usages;
    count(usages, in);
    std::vector<node> tree;
    build_tree(tree, file_size, number_of_words, usages);
    write(out, reinterpret_cast<uint8_t *>(&file_size), 8, hash);
    std::vector<std::pair<uint64_t, uint8_t>> translate(1u << 8u);
    std::vector<char> tree_structure;
    std::vector<uint8_t> words;
    dfs(static_cast<uint16_t>(tree.size() - 1), 0, tree, translate, 0, tree_structure, words);
    write(out, reinterpret_cast<uint8_t *>(&number_of_words), 2, hash);
    for (auto word : words) {
        write(out, &word, 1, hash);
    }
    write_tree(out, tree_structure, hash);
    encode_file(in, out, hash, translate);
    out.seekp(0, std::ios::beg);
    out.write(reinterpret_cast<const char *>(&hash), 8);
}

void huffman::decode(std::istream &in, std::ostream &out) {
    if (is_empty(in)) {
        return;
    }
    uint8_t buffer[1024 * 1024];
    if (!check_hash(in, buffer)) {
        throw std::runtime_error("file is corrupted");
    }
    uint64_t file_size = 0;
    in.read(reinterpret_cast<char *>(&file_size), 8);
    uint16_t num_of_words = 0;
    in.read(reinterpret_cast<char *>(&num_of_words), 2);
    char words[1u << 8u];
    in.read(words, num_of_words);
    std::vector<simple_node> tree;
    read_tree(in, tree, buffer, words);
    decode_file(in, out, tree, buffer, words, file_size);
}