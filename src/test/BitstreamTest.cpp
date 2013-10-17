#include <boost/test/unit_test.hpp>

#include <iostream>
#include <chrono>
#include <fstream>
#include <boost/dynamic_bitset.hpp>

#include "Bitstream.hpp"

const auto writes = 1e3;

BOOST_AUTO_TEST_CASE(test_boost_dynamic_bitset)
{
    std::cout << "\n>>> test_boost_dynamic_bitset\n";
    using namespace std::chrono;

    boost::dynamic_bitset<> bitset;
    srand((unsigned int) time(NULL));

    bool val = false;
    auto start = high_resolution_clock::now();
    for (int x = 0; x < writes; ++x) {
        if (!(x % 4)) val = !val;
        bitset.push_back(val);
    }
    auto end = high_resolution_clock::now();

    std::cout << "Adding " << writes << " bits: " << duration_cast<milliseconds>(end - start).count() << " ms\n";
    std::cout << "Bitset size: " << bitset.size() << " elements\n";

    // reading all bits
    start = high_resolution_clock::now();
    unsigned long long sum = 0;
    for (auto i = 0U; i < bitset.size(); ++i) {
        sum += bitset[i];
    }
    end = high_resolution_clock::now();
    std::cout << "Counted " << sum << " active bits in " << duration_cast<milliseconds>(end - start).count() << " ms\n";

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
    BOOST_CHECK_EQUAL(in[3], true);
    BOOST_CHECK_EQUAL(in[4], false);
    BOOST_CHECK_EQUAL(in[7], false);
    BOOST_CHECK_EQUAL(in[8], true);
    BOOST_CHECK_EQUAL(in[11], true);
    BOOST_CHECK_EQUAL(in[12], false);
    BOOST_CHECK_EQUAL(in.size(), writes);
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
        if (!(x % 4)) val = !val;
        bitset << val; // or bitset.push_back(val)
    }
    auto end = high_resolution_clock::now();

    std::cout << "Adding " << writes << " bits: " << duration_cast<milliseconds>(end - start).count() << " ms\n";
    std::cout << "Bitset size: " << bitset.size() << " elements\n";

    // reading all bits
    start = high_resolution_clock::now();
    unsigned long long sum = 0;
    for (auto i = 0U; i < bitset.size(); ++i) {
        sum += bitset[i];
    }
    end = high_resolution_clock::now();
    std::cout << "Counted " << sum << " active bits in " << duration_cast<milliseconds>(end - start).count() << " ms\n";

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
    BOOST_CHECK_EQUAL(in[3], true);
    BOOST_CHECK_EQUAL(in[4], false);
    BOOST_CHECK_EQUAL(in[7], false);
    BOOST_CHECK_EQUAL(in[8], true);
    BOOST_CHECK_EQUAL(in[11], true);
    BOOST_CHECK_EQUAL(in[12], false);
    BOOST_CHECK_EQUAL(in.size(), writes);
}
