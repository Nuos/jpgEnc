#include <boost/test/unit_test.hpp>

#include <iostream>
#include <chrono>
#include <fstream>
#include <boost/dynamic_bitset.hpp>

#include "Bitstream.hpp"

const auto writes = 1e3;

BOOST_AUTO_TEST_CASE(test_own_bitstream)
{
    // default constructor
    Bitstream def;
    BOOST_CHECK_EQUAL(def.size(), 0);

    // initializer list constructor
    Bitstream b0{1, 0, 0, 1, 1, 1, 1, 0, 0, 1};
    BOOST_CHECK(b0[0] == true);
    BOOST_CHECK(b0[2] == false);
    BOOST_CHECK(b0[4] == true);
    BOOST_CHECK(b0[8] == false);
    BOOST_CHECK(b0[9] == true);
    BOOST_CHECK_EQUAL(b0.size(), 10);

    // assigning bits
    b0[0] = false;
    b0[8] = 1;
    BOOST_CHECK(b0[0] == false);
    BOOST_CHECK(b0[8] == true);
    BOOST_CHECK_EQUAL(b0.size(), 10);

    // appending initializer list
    b0 << Bits{1, 0, 0, 1};
    BOOST_CHECK(b0[10] == true);
    BOOST_CHECK(b0[11] == false);
    BOOST_CHECK(b0[12] == false);
    BOOST_CHECK(b0[13] == true);
    BOOST_CHECK_EQUAL(b0.size(), 14);

    // appending single bits
    b0 << false << true << true << true;
    BOOST_CHECK(b0[14] == false);
    BOOST_CHECK(b0[15] == true);
    BOOST_CHECK(b0[16] == true);
    BOOST_CHECK(b0[17] == true);
    BOOST_CHECK_EQUAL(b0.size(), 18);

    Bitstream bitset;
    srand((unsigned int) time(NULL));

    bool val = false;
    for (int x = 0; x < writes; ++x) {
        if (!(x % 4)) val = !val;
        bitset << val; // or bitset.push_back(val)
    }
    BOOST_CHECK_EQUAL(bitset.size(), writes);

    {
        {
            std::ofstream bitset_out("saved_bitset", std::fstream::binary);
            bitset_out << bitset;
        }

        Bitstream in;
        {
            std::ifstream bitset_in("saved_bitset", std::fstream::binary);
            bitset_in >> in;
        }

        BOOST_CHECK_EQUAL(in[0], true);
        BOOST_CHECK_EQUAL(in[3], true);
        BOOST_CHECK_EQUAL(in[4], false);
        BOOST_CHECK_EQUAL(in[7], false);
        BOOST_CHECK_EQUAL(in[8], true);
        BOOST_CHECK_EQUAL(in[11], true);
        BOOST_CHECK_EQUAL(in[12], false);
        BOOST_CHECK_EQUAL(in.size(), writes);
    }
}
