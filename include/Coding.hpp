#pragma once

#include <vector>
#include <cassert>
#include <utility>

#include "BitstreamGeneric.hpp"

struct RLE_PAIR {
    short num_zeros_before : 4;
    int value;

    RLE_PAIR(short zeros, int _value) : num_zeros_before(zeros), value(_value) { assert(zeros <= 16); }
};

bool operator==(const RLE_PAIR &left, const RLE_PAIR &right) {
    return (left.num_zeros_before == right.num_zeros_before) && (left.value == right.value);
}

// takes the full, zigzag sorted value list with DC and AC data, first entry is DC component and will be ignored
inline std::vector<RLE_PAIR> RLE_AC(const std::vector<int> &data) {
    //assert(data.size() == 64);

    std::vector<RLE_PAIR> AC_rle;

    unsigned int zero_counter = 0;
    for (auto it = begin(data) + 1; it != end(data); ++it) {
        const auto& value = *it;

        if (value == 0) {
            ++zero_counter;
            if (zero_counter > 15) {
                AC_rle.push_back(RLE_PAIR(15, 0));
                zero_counter = 0;
            }
            continue;
        }
        else {
            AC_rle.push_back(RLE_PAIR(zero_counter, value));
            zero_counter = 0;
        }
    }

    // EOB
    if (zero_counter > 0)
        AC_rle.push_back(RLE_PAIR(0, 0));

    return AC_rle;
}

struct Category_Code {
    uint8_t symbol;
    Bitstream code;

    Category_Code(uint8_t p, Bitstream b) : symbol(p), code(b) {}
    ~Category_Code() {}
};

bool operator==(const Category_Code &left, const Category_Code &right) {
    return (left.symbol == right.symbol) && (left.code == right.code);
}


std::pair<short, Bitstream> getCategoryAndCode(int value) {
    if (value == 0)
        return std::make_pair(0, Bitstream());

    unsigned short category = 1;
    auto bound = 2l;
    while (category < 16) {
        auto upper_bound = bound - 1;
        auto lower_bound = (bound >> 1);
        auto bound_diff = upper_bound - lower_bound;

        auto abs_val = abs(value);
        if (abs_val >= lower_bound && abs_val <= upper_bound) {
            // got category, generate code

            auto offset = 0l;
            if (value < 0)
                offset = (upper_bound - abs_val);
            else
                offset = (bound_diff + 1) + value - lower_bound;

            return std::make_pair(category, Bitstream(offset, category));
        }
        
        bound <<= 1;
        ++category;
    }

    assert(!"Shouldn't happen!");
}

// takes the encoded list of RLE_PAIRS and generates the symbol for huffman coding and a code from category encoding 
inline std::vector<Category_Code> encode_category(const std::vector<RLE_PAIR> &data) {
    std::vector<Category_Code> category_list;

    for (const auto& rle_pair : data) {
        auto data = getCategoryAndCode(rle_pair.value);

        auto category = data.first;
        auto &code = data.second;

        assert(rle_pair.num_zeros_before < 16);
        assert(category < 16);
        auto symbol = (rle_pair.num_zeros_before << 4) | category;

        category_list.emplace_back(symbol, code);
    }

    return category_list;
}