#pragma once

#include "BitstreamGeneric.hpp"

#include <vector>
#include <memory>
#include <unordered_map>
#include <queue>
#include <iostream>
#include <assert.h>

using std::unordered_map;
using std::priority_queue;
using std::vector;


class Node {
public:
    Node(double probability, Node* left, Node* right);
    Node(double probability, int symbol);
    ~Node();

    // longest path to a child
    int height() const;

public:
    double probability;

    // bit hacky for now..
    int symbol;
    bool is_leaf;

    Node* left;
    Node* right;
};

// a huffman code for a symbol
struct Code {
    Code() : code(0), length(0) {}
    Code(uint32_t code, uint8_t length)
        : code(code), length(length)
    {
        // make the leftmost bit of the code the MSB
        auto shift = sizeof(uint32_t)* 8 - length;
        this->code <<= shift;
    }

    Code(Bitstream bitstream) {
        length = bitstream.size();
        assert(length < 64);

        code = bitstream.extract(length, 0);
    }

    // like bistreams, the code begins at the msb
    uint32_t code;
    uint8_t length;
};

using SymbolCodeMap = std::unordered_map<int, Code>;


// takes a text, caluclates the probability of every symbol and returns a map from symbols to huffman codes
SymbolCodeMap generate_code_map(std::vector<int> text);

Node* generate_huff_tree(unordered_map<int, int> symbol_counts, int total_symbols);
// generate a list of symbols grouped by code length
void generate_symbols_per_codelength(Node* node, int depth, vector<vector<int>>& symbols);
SymbolCodeMap generate_codes(vector<vector<int>>& symbols);


// en- and decoding
Bitstream huffman_encode(vector<int> text, SymbolCodeMap code_map);
vector<int> huffman_decode(Bitstream bitstream, SymbolCodeMap code_map);

struct DecodeEntry {
    DecodeEntry(uint32_t code, uint8_t code_length, int symbol);

    // code starting at msb, rest filled with 1s
    uint32_t code;
    uint8_t code_length;
    int symbol;

    bool operator<(const DecodeEntry& other) {
        return code < other.code;
    }
};