#include <boost/test/unit_test.hpp>

#include "Huffman.hpp"

BOOST_AUTO_TEST_CASE(package_test) {
    Package p1{ Symbol(1, 5) };
    Package p2{ Symbol(5, 8) };
    Package p3{ p1, p2 };

    BOOST_CHECK_EQUAL(5, p1.weight);
    BOOST_CHECK_EQUAL(8, p2.weight);
    BOOST_CHECK_EQUAL(13, p3.weight);
}

BOOST_AUTO_TEST_CASE(package_merge_test) {
    for (auto i = 0; i < 50000; ++i) {

    vector<Symbol> symbols{
        Symbol(0, 6),
        Symbol(4, 20),
        Symbol(1, 3),
        Symbol(9, 24),
        Symbol(7, 1)
    };
    
    auto code_lengths = package_merge(symbols, 5);
    BOOST_CHECK_EQUAL(4, code_lengths[7]);
    BOOST_CHECK_EQUAL(4, code_lengths[1]);
    BOOST_CHECK_EQUAL(3, code_lengths[0]);
    BOOST_CHECK_EQUAL(2, code_lengths[4]);
    BOOST_CHECK_EQUAL(1, code_lengths[9]);

    code_lengths = package_merge(symbols, 3);
    BOOST_CHECK_EQUAL(3, code_lengths[7]);
    BOOST_CHECK_EQUAL(3, code_lengths[1]);
    BOOST_CHECK_EQUAL(2, code_lengths[0]);
    BOOST_CHECK_EQUAL(2, code_lengths[4]);
    BOOST_CHECK_EQUAL(2, code_lengths[9]);
    }
}