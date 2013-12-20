#include "test/unittest.hpp"

#include "Coding.hpp"

BOOST_AUTO_TEST_CASE(rle_AC_test) {
    std::vector<int> data{ 1, 57, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 2 };
    std::vector<RLE_PAIR> expected_rle_data{ RLE_PAIR(0, 57), RLE_PAIR(15, 0), RLE_PAIR(2, 3), RLE_PAIR(4, 2) };

    std::vector<int> zero_end_data{ 1, 57, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 2, 0, 0 };
    std::vector<RLE_PAIR> expected_rle_zero_end_data{ RLE_PAIR(0, 57), RLE_PAIR(15, 0), RLE_PAIR(2, 3), RLE_PAIR(4, 2), RLE_PAIR(0, 0) };

    auto rle_data = RLE_AC(data);
    auto rle_zero_end_data = RLE_AC(zero_end_data);
    
    BOOST_CHECK(expected_rle_data == rle_data);
    BOOST_CHECK(expected_rle_zero_end_data == rle_zero_end_data);
}
