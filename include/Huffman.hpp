#pragma once

#include "BitstreamGeneric.hpp"

#include <vector>
#include <memory>
#include <unordered_map>
#include <queue>
#include <iostream>

using std::unordered_map;
using std::priority_queue;
using std::vector;

struct Code {
    Code() : code(0), length(0) {}
    Code(uint32_t code, uint8_t length)
        : code(code), length(length)
    {}

    Code(Bitstream bitstream) {
        length = bitstream.size();
        assert(length < 64);

        code = bitstream.extract(length, 0);
    }

    uint32_t code;
    uint8_t length;
};

using SymbolCodeMap = std::unordered_map<int, Code>;


class Node {
public:
    Node(double probability, Node* left, Node* right)
        : probability(probability), 
          left(left), 
          right(right),
          is_leaf(false)
    {}

    Node(double probability, int symbol)
        : probability(probability), 
          symbol(symbol),
          is_leaf(true),
          left(nullptr),
          right(nullptr)
    {}
    ~Node() {
        delete left;
        delete right;
    }

    // longest path to a child
    // easily cacheable...
    int height() const {
        if (is_leaf)
            return 0;
        else if (left && right)
            return std::max(left->height(), right->height()) + 1;
        else if (left)
            return left->height() + 1;
        else
            return right->height() + 1;
    }

public:
    double probability;

    // bit hacky for now..
    int symbol;
    bool is_leaf;

    Node* left;
    Node* right;
};


Node* generate_huff_tree(unordered_map<int, int> symbol_counts, int total) {
    auto comp = [](const Node* lhs, const Node* rhs) {
        return lhs->probability > rhs->probability;
    };
    priority_queue<Node*, vector<Node*>, decltype(comp)> nodes;

    // create a leaf node for every symbol
    // also calculate it's probability from the given count and total
    for (const auto& pair : symbol_counts) {
        // pair.first: symbol
        // pair.second: count
        nodes.emplace(new Node((double)pair.second / (double)total, pair.first));
    }

    // combine the two nodes with lowest probability into a new one till only one is left
    while (nodes.size() > 1) {
        // least probable symbol or node
        Node* first = nodes.top();
        nodes.pop();
        // second least probable symbol or node
        Node* second = nodes.top();
        nodes.pop();

        // always put the higher tree to the right
        if (first->height() > second->height()) {
            std::swap(first, second);
        }
        nodes.emplace(new Node(first->probability + second->probability, first, second));
    }
    
    Node* root = nodes.top();
    // handle the case where we only have a single symbol
    if (root->is_leaf)
        return new Node(root->probability, root, nullptr);
    else
        return root;
}


void replace_rightmost_leaf(Node* root) {
    assert(root != nullptr);

    Node* node = root;
    Node* parent = nullptr;

    while (node->right) {
        parent = node;
        node = node->right;
    }

    if (!parent || !node->is_leaf)
        return;

    // replace rightmost leaf with new node and leaf as left child
    parent->right = new Node(node->probability, node, nullptr);
}


void generate_codes(Node* node, Bitstream& prefix, SymbolCodeMap& code_map) {
    if (node->is_leaf) {
        // arrived at leaf, so we are done and save the huffman code
        code_map[node->symbol] = Code(prefix);
    }
    else {
        if (node->left) {
            // create new code prefix for the left part of the tree
            Bitstream new_prefix = prefix;
            new_prefix << false;
            generate_codes(node->left, new_prefix, code_map);
        }
        if (node->right) {
            // no copy of prefix, as it isn't used anywhere anymore
            prefix << true;
            generate_codes(node->right, prefix, code_map);

        }
    }
}


SymbolCodeMap generate_code_map(std::vector<int> text) {
    assert(text.size() > 0);

    unordered_map<int, int> symbol_counts;
    for (auto& symbol : text) {
        if (symbol_counts.count(symbol) == 0) {
            symbol_counts.emplace(symbol, 1);
        }
        else {
            ++symbol_counts[symbol];
        }
    }

    Node* root = generate_huff_tree(symbol_counts, text.size());
    // prevents a huffman code consiting of only 1s
    replace_rightmost_leaf(root);

    SymbolCodeMap code_map;
    generate_codes(root, Bitstream(), code_map);

    delete root;
    return code_map;
}


Bitstream huffman_encode(vector<int> text, SymbolCodeMap code_map) {
    Bitstream result;
    for (auto symbol : text) {
        const Code& code = code_map[symbol];
        result.push_back(code.code, code.length);
    }
    return result;
}

// fill everything to the right of the code with 1s
uint32_t fill_rest_with_ones(uint32_t code, uint8_t code_length) {
    return code | ((1 << (32 - code_length)) - 1);
}

struct DecodeEntry {
    DecodeEntry(uint32_t code, uint8_t code_length, int symbol) {
        this->symbol = symbol;
        this->code_length = code_length;
        this->code = fill_rest_with_ones(code, code_length);
    }

    // code starting at msb, rest filled with 1s
    uint32_t code;
    uint8_t code_length;
    int symbol;

    bool operator<(const DecodeEntry& other) {
        return code < other.code;
    }
};

vector<int> huffman_decode(Bitstream bitstream, SymbolCodeMap code_map) {
    vector<DecodeEntry> code_table;
    code_table.reserve(code_map.size());

    uint8_t max_code_length = 0;

    // prepare code_table and calc max_code_length
    for (const std::pair<int, Code>& pair : code_map) {
        uint8_t code_length = pair.second.length;
        if (code_length > max_code_length)
            max_code_length = code_length;

        code_table.emplace_back(pair.second.code, code_length, pair.first);
    }
    assert(max_code_length <= 32);

    // sort accoring to code
    std::sort(code_table.begin(), code_table.end());


    vector<int> decoded_text;
    uint8_t to_extract = max_code_length;

    // current pos in bitstream
    unsigned int pos = 0;

    while (pos < bitstream.size()) {
        // when we are at the end, we have to extract less
        if (pos + max_code_length >= bitstream.size())
            to_extract = bitstream.size() - pos;

        uint32_t bits = bitstream.extract(to_extract, pos);
        bits = fill_rest_with_ones(bits, max_code_length);

        // find matching symbol for extracted code
        // todo: binary search
        // linear search for now...
        int index = -1;
        for (int i = 0; i < code_table.size(); i++) {
            // first entry that is greater than the extracted bits
            if (code_table[i].code >= bits) {
                index = i;
                break;
            }
        }
        // no entry found
        // some better error handling?
        assert(index != -1);

        decoded_text.push_back(code_table[index].symbol);

        // next position to extract from
        pos += code_table[index].code_length;
    }

    return decoded_text;
}