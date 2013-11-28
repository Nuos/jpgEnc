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

    vector<Symbol> symbols{
        Symbol(0, 6),
        Symbol(4, 20),
        Symbol(1, 3),
        Symbol(9, 24),
        Symbol(7, 1)
    };
    
    auto code_lengths = package_merge(symbols, 5);

    // BOOST_CHECK_EQUAL_COLLECTIONS doesn't check content order

    auto length_one = vector<int>{{ 9 }};
    auto length_two = vector<int>{{ 4 }};
    auto length_three = vector<int>{{ 0 }};
    auto length_four = vector<int>{{ 1, 7}};
    BOOST_CHECK_EQUAL_COLLECTIONS(begin(length_one),   end(length_one),   begin(code_lengths[1]), end(code_lengths[1]));
    BOOST_CHECK_EQUAL_COLLECTIONS(begin(length_two),   end(length_two),   begin(code_lengths[2]), end(code_lengths[2]));
    BOOST_CHECK_EQUAL_COLLECTIONS(begin(length_three), end(length_three), begin(code_lengths[3]), end(code_lengths[3]));
    BOOST_CHECK_EQUAL_COLLECTIONS(begin(length_four),  end(length_four),  begin(code_lengths[4]), end(code_lengths[4]));

    code_lengths = package_merge(symbols, 3);

    length_two = vector<int>{{ 4, 0, 9 }};
    length_three = vector<int>{{ 1, 7 }};
    BOOST_CHECK_EQUAL_COLLECTIONS(begin(length_two),   end(length_two),   begin(code_lengths[2]), end(code_lengths[2]));
    BOOST_CHECK_EQUAL_COLLECTIONS(begin(length_three), end(length_three), begin(code_lengths[3]), end(code_lengths[3]));
}