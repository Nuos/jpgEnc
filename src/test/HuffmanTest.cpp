#include <boost/test/unit_test.hpp>
#include <vector>
#include "BitstreamGeneric.hpp"

#include "Huffman.hpp"

BOOST_AUTO_TEST_CASE(test_tree) {
    vector<int> text{ 4, 4, 4, 4, 2, 2, 1 };
    CodeMap code_map = generate_code_map(text);
    BOOST_CHECK_EQUAL(code_map[1], Bitstream({ 0, 0 }));
    BOOST_CHECK_EQUAL(code_map[2], Bitstream({ 1, 0 })); // tja, wie rum wollmers?
    BOOST_CHECK_EQUAL(code_map[4], Bitstream({ 1 }));
    BOOST_CHECK_EQUAL(code_map.size(), 3);
}
