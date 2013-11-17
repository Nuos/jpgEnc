#include <boost/test/unit_test.hpp>
#include <vector>
#include "BitstreamGeneric.hpp"

#include "Huffman.hpp"

BOOST_AUTO_TEST_CASE(test_tree) {
    vector<int> text{5,5,5,5,5, 4,4,4,4, 2,2, 1 };
    CodeMap code_map = generate_code_map(text);

    // The root of a Huffman code is placed toward the MSB of the byte, 
    // and successive bits are placed in the direction MSB to LSB of the byte
    BOOST_CHECK_EQUAL(code_map[1], Bitstream({ 1, 1, 0 }));
    BOOST_CHECK_EQUAL(code_map[2], Bitstream({ 1, 1, 1, 0 }));
    BOOST_CHECK_EQUAL(code_map[4], Bitstream({ 1, 0 }));
    BOOST_CHECK_EQUAL(code_map[5], Bitstream({ 0 }));
    BOOST_CHECK_EQUAL(code_map.size(), 4);
}
