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
using CodeMap = std::unordered_map<int, Bitstream>;


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
    return nodes.top();
}


void replace_rightmost_leaf(Node* root) {
    assert(root != nullptr);
    assert(!root->is_leaf);

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


void generate_codes(Node* node, Bitstream& prefix, CodeMap& code_map) {
    if (node->is_leaf) {
        // arrived at leaf, so we are done and save the huffman code
        code_map[node->symbol] = prefix;
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


CodeMap generate_code_map(std::vector<int> text) {
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
    replace_rightmost_leaf(root);

    CodeMap code_map;
    generate_codes(root, Bitstream(), code_map);

    delete root;
    return code_map;
}