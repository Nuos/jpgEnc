#pragma once

#include <vector>
#include <cassert>
#include <utility>

#include <boost/numeric/ublas/matrix.hpp>

#include "BitstreamGeneric.hpp"

using boost::numeric::ublas::matrix;
typedef double PixelDataType;
using mat = matrix<PixelDataType>;
typedef unsigned int uint;
typedef uint8_t Byte;

template <typename T>
matrix<T> from_vector(const std::vector<T>& v) {
    assert(v.size() == 64);

    matrix<T> m(8, 8);
    for (size_t i = 0; i < m.size1(); i++) {
        for (size_t j = 0; j < m.size2(); j++) {
            m(i, j) = v[i*m.size2() + j];
        }
    }
    return m;
}

template <typename T>
std::vector<T> zigzag(matrix<T> m) {
    const auto lookup = from_vector<uint>({
        0, 1, 5, 6, 14, 15, 27, 28,
        2, 4, 7, 13, 16, 26, 29, 42,
        3, 8, 12, 17, 25, 30, 41, 43,
        9, 11, 18, 24, 31, 40, 44, 53,
        10, 19, 23, 32, 39, 45, 52, 54,
        20, 22, 33, 38, 46, 51, 55, 60,
        21, 34, 37, 47, 50, 56, 59, 61,
        35, 36, 48, 49, 57, 58, 62, 63
    });

    assert(m.size1() == 8);
    assert(m.size2() == 8);
    std::vector<T> r;
    r.resize(64);

    for (uint i = 0; i < 8; ++i) {
        for (uint j = 0; j < 8; ++j)
            r[lookup(i, j)] = m(i, j);
    }

    return r;
};

inline matrix<int> quantize(const mat& m, const mat& table) {
    assert(m.size1() == 8);
    assert(m.size2() == 8);
    assert(table.size1() == 8);
    assert(table.size2() == 8);

    matrix<int> result(8, 8);

    for (uint i = 0; i < 64; ++i) {
        result.data()[i] = static_cast<int>(std::round(m.data()[i] / table.data()[i]));
    }

    return result;
}

struct RLE_PAIR {
    unsigned short num_zeros_before : 4;
    int value;

    RLE_PAIR(short zeros, int _value) : num_zeros_before(zeros), value(_value) { assert(zeros <= 16); }
};

inline bool operator==(const RLE_PAIR &left, const RLE_PAIR &right) {
    return (left.num_zeros_before == right.num_zeros_before) && (left.value == right.value);
}

// takes the full, zigzag sorted value list with DC and AC data, first entry is DC component and will be ignored
inline std::vector<RLE_PAIR> RLE_AC(const std::vector<int> &data) {
    assert(data.size() > 1);

    std::vector<RLE_PAIR> AC_rle;

    AC_rle.push_back(RLE_PAIR(0, data[0]));

    unsigned int zero_counter = 0;
    for (auto it = begin(data) + 1; it != end(data); ++it) {
        const auto& value = *it;

        if (value == 0) {
            ++zero_counter;
            continue;
        }
        else {
            if (zero_counter > 15) {
                do {
                    AC_rle.push_back(RLE_PAIR(15, 0));
                    zero_counter -= 16;
                } while (zero_counter > 15);
            }
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

inline bool operator==(const Category_Code &left, const Category_Code &right) {
    return (left.symbol == right.symbol) && (left.code == right.code);
}


inline std::pair<short, Bitstream> getCategoryAndCode(int value) {
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
                offset = upper_bound - abs_val;
            else
                offset = value;

            return std::make_pair(category, Bitstream(offset, category));
        }
        
        bound <<= 1;
        ++category;
    }

    assert(!"Shouldn't happen!");
    return std::make_pair(0, Bitstream());
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