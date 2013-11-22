#include "Huffman.hpp"

Node::Node(double probability, Node* left, Node* right)
    : probability(probability),
    left(left),
    right(right),
    is_leaf(false)
{}

Node::Node(double probability, int symbol)
    : probability(probability),
    symbol(symbol),
    is_leaf(true),
    left(nullptr),
    right(nullptr)
{}

Node::~Node() {
    delete left;
    delete right;
}

int Node::height() const {
    if (is_leaf)
        return 0;
    else if (left && right)
        return std::max(left->height(), right->height()) + 1;
    else if (left)
        return left->height() + 1;
    else
        return right->height() + 1;
}


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

        nodes.emplace(new Node(first->probability + second->probability, first, second));
    }

    Node* root = nodes.top();
    // handle the case where we only have a single symbol
    if (root->is_leaf)
        return new Node(root->probability, root, nullptr);
    else
        return root;
}


void generate_symbols_per_codelength(Node* node, int depth, vector<vector<int>>& symbols) {
    if (node->is_leaf) {
        symbols[depth].push_back(node->symbol);
    }
    else {
        if (node->left) {
            generate_symbols_per_codelength(node->left, depth + 1, symbols);
        }
        if (node->right) {
            generate_symbols_per_codelength(node->right, depth + 1, symbols);
        }
    }
}


SymbolCodeMap generate_codes(vector<vector<int>>& symbols) {
    // based on the algorithm in
    // Reza Hashemian: Memory Efficient and High-speed Search Huffman Coding, 1995

    SymbolCodeMap code_map;

    uint32_t code = 0;
    for (int length = 1; length < symbols.size(); length++) {
        for (int symbol : symbols[length]) {
            if (code == (1 << length) - 1) {
                // the very last code, prevent a code consisting of only 1s
                code_map[symbol] = Code(code << 1, length+1);
            }
            else {
                code_map[symbol] = Code(code, length);
            }
            ++code;
        }
        code <<= 1;
    }

    return code_map;
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

    // list of symbols grouped by code length
    // symbols_per_length = symbols[code_length]
    vector<vector<int>> symbols(root->height()+1);
    generate_symbols_per_codelength(root, 0, symbols);

    SymbolCodeMap code_map = generate_codes(symbols);

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

DecodeEntry::DecodeEntry(uint32_t code, uint8_t code_length, int symbol) {
    this->symbol = symbol;
    this->code_length = code_length;
    this->code = fill_rest_with_ones(code, code_length);
}

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