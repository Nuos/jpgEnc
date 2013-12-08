#include "test/unittest.hpp"

#include <vector>

#include "BitstreamGeneric.hpp"
#include "Huffman.hpp"

bool equals(Code code, Bitstream stream) {
    Code code2{ stream };
    return code.code == code2.code && code.length == code2.length;
}

BOOST_AUTO_TEST_CASE(test_tree) {
    vector<int> text{5,5,5,5,5, 4,4,4,4, 2,2, 1 };
    SymbolCodeMap code_map = generateCodeMap(text);

    // The root of a Huffman code is placed toward the MSB of the byte, 
    // and successive bits are placed in the direction MSB to LSB of the byte
    BOOST_CHECK(equals(code_map[1], Bitstream({ 1, 1, 0 })));
    BOOST_CHECK(equals(code_map[2], Bitstream({ 1, 1, 1, 0 })));
    BOOST_CHECK(equals(code_map[4], Bitstream({ 1, 0 })));
    BOOST_CHECK(equals(code_map[5], Bitstream({ 0 })));
    BOOST_CHECK_EQUAL(code_map.size(), 4);

    SymbolCodeMap code_map2 = generateCodeMap({ 1 });
    BOOST_CHECK(equals(code_map2[1], Bitstream({ 0 })));
    BOOST_CHECK_EQUAL(code_map2.size(), 1);
}

BOOST_AUTO_TEST_CASE(test_tree_right_growing) {
    vector<int> text{2,2,22,22, 5,5,5,5,5, 3,3,3,33,33,33, 7,7,7,7,7,7,7};
    SymbolCodeMap code_map = generateCodeMap(text);

    BOOST_CHECK(equals(code_map[22], Bitstream({ 1, 0, 0 })));
    BOOST_CHECK(equals(code_map[2],  Bitstream({ 1, 0, 1 })));
    BOOST_CHECK(equals(code_map[33], Bitstream({ 1, 1, 1, 0 })));
    BOOST_CHECK(equals(code_map[3],  Bitstream({ 1, 1, 0 })));
    BOOST_CHECK(equals(code_map[5],  Bitstream({ 0, 0 })));
    BOOST_CHECK(equals(code_map[7],  Bitstream({ 0, 1 })));
    BOOST_CHECK_EQUAL(code_map.size(), 6);
}

BOOST_AUTO_TEST_CASE(encoding) {
    vector<int> text{ 5, 5, 5, 5, 5, 4, 4, 4, 4, 2, 2, 1 };
    SymbolCodeMap code_map = generateCodeMap(text);

    Bitstream encoded = huffmanEncode(text, code_map);
    Bitstream should{
        0, 0, 0, 0, 0,
        1,0, 1,0, 1,0, 1,0,
        1,1,1,0, 1,1,1,0,
        1,1,0
    };
    BOOST_CHECK_EQUAL(encoded, should);
}

BOOST_AUTO_TEST_CASE(decoding) {
    vector<int> text{ 4, 4, 4, 4, 2, 2, 1, 5, 5, 5, 5, 5};
    SymbolCodeMap code_map = generateCodeMap(text);

    Bitstream encoded = huffmanEncode(text, code_map);

    vector<int> decoded = huffmanDecode(encoded, code_map);
    BOOST_CHECK(text == decoded);
}

BOOST_AUTO_TEST_CASE(decoding2) {
    vector<int> text{ 4, 4, 4, 4, 2, 2, 1, 5, 5, 5, 5, 5,3,5,6,5,3,4,5,6,1010,203,4,0,111111};
    SymbolCodeMap code_map = generateCodeMap(text);

    Bitstream encoded = huffmanEncode(text, code_map);

    vector<int> decoded = huffmanDecode(encoded, code_map);
    BOOST_CHECK(text == decoded);
}