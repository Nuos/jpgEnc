#pragma once

#include <vector>
#include <cassert>

struct RLE_PAIR {
    int num_zeros_before;
    int value;

    RLE_PAIR(int zeros, int _value) : num_zeros_before(zeros), value(_value) {}
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