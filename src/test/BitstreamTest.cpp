#include <boost/test/unit_test.hpp>

#include <iostream>
#include <chrono>
#include <fstream>
#include <boost/dynamic_bitset.hpp>

#include "Bitstream.hpp"

const auto writes = 1e7;

BOOST_AUTO_TEST_CASE(test_boost_dynamic_bitset)
{
    std::cout << "\n>>> test_boost_dynamic_bitset\n";
    using namespace std::chrono;

    boost::dynamic_bitset<> bitset;
    srand((unsigned int) time(NULL));

    bool val = false;
    auto start = high_resolution_clock::now();
    for (int x = 0; x < writes; ++x) {
        if (!(x % 5)) val = !val;
        bitset.push_back(val);
    }
    auto end = high_resolution_clock::now();

    std::cout << "Adding " << writes << " bits: " << duration_cast<milliseconds>(end - start).count() << " ms\n";
    std::cout << "Bitset size: " << bitset.size() << " elements\n";

    start = high_resolution_clock::now();
    {
        std::ofstream bitset_out("saved_bitset", std::fstream::binary);
        bitset_out << bitset;
    }

    boost::dynamic_bitset<> in;
    {
        std::ifstream bitset_in("saved_bitset", std::fstream::binary);
        bitset_in >> in;
    }
    end = high_resolution_clock::now();

    std::cout << "Writing to and reading from file: " << duration_cast<milliseconds>(end - start).count() << " ms\n";

    BOOST_CHECK_EQUAL(in[0], true);
    BOOST_CHECK_EQUAL(in[4], true);
    BOOST_CHECK_EQUAL(in[5], false);
    BOOST_CHECK_EQUAL(in[9], false);
    BOOST_CHECK_EQUAL(in[10], true);
}


BOOST_AUTO_TEST_CASE(test_own_bitstream)
{
    std::cout << "\n>>> test_own_bitstream\n";
    using namespace std::chrono;

    Bitstream bitset;
    srand((unsigned int) time(NULL));

    bool val = false;
    auto start = high_resolution_clock::now();
    for (int x = 0; x < writes; ++x) {
        if (!(x % 5)) val = !val;
        bitset << val; // or bitset.push_back(val)
    }
    auto end = high_resolution_clock::now();

    std::cout << "Adding " << writes << " bits: " << duration_cast<milliseconds>(end - start).count() << " ms\n";
    std::cout << "Bitset size: " << bitset.size() << " elements\n";

    start = high_resolution_clock::now();
    {
        std::ofstream bitset_out("saved_bitset", std::fstream::binary);
        bitset_out << bitset;
    }

    Bitstream in;
    {
        std::ifstream bitset_in("saved_bitset", std::fstream::binary);
        bitset_in >> in;
    }
    end = high_resolution_clock::now();

    std::cout << "Writing to and reading from file: " << duration_cast<milliseconds>(end - start).count() << " ms\n";

    BOOST_CHECK_EQUAL(in[0], true);
    BOOST_CHECK_EQUAL(in[4], true);
    BOOST_CHECK_EQUAL(in[5], false);
    BOOST_CHECK_EQUAL(in[9], false);
    BOOST_CHECK_EQUAL(in[10], true);
}
