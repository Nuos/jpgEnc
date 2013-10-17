#include <boost/test/unit_test.hpp>

#include <iostream>
#include <chrono>
#include <fstream>
#include <boost/dynamic_bitset.hpp>

#include "BitstreamLong.hpp"

#if _DEBUG
const auto writes = 1e4;
#else
const auto writes = 1e7;
#endif

BOOST_AUTO_TEST_CASE(test_own_bitstream_long)
{
    std::cout << "\n>>> test_own_bitstream_long\n";
    using namespace std::chrono;

    // initializer list ctor
    BitstreamL init{0, 1, 1, 0, 1, 0, 1, 0, 1, 1};

    // appending bits
    BitstreamL bitset;
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

    start = high_resolution_clock::now();
    {
        std::ofstream bitset_out("saved_bitset", std::fstream::binary);
        bitset_out << bitset;
    }

    BitstreamL in;
    {
        std::ifstream bitset_in("saved_bitset", std::fstream::binary);
        bitset_in >> in;
    }
    end = high_resolution_clock::now();

    std::cout << "Writing to and reading from file: " << duration_cast<milliseconds>(end - start).count() << " ms\n";

    // calculate real written bits to file as 64bit isn't aligned with 1e4 bits and we write only full blocks
    auto lwrites = (long long) writes;
    auto aligned = (0 == lwrites - ((lwrites / BitstreamL::block_size) * BitstreamL::block_size));
    auto real_writes = aligned ? lwrites : ((lwrites / BitstreamL::block_size) * BitstreamL::block_size + BitstreamL::block_size);

    BOOST_CHECK(in[0] == true);
    BOOST_CHECK(in[3] == true);
    BOOST_CHECK(in[4] == false);
    BOOST_CHECK(in[7] == false);
    BOOST_CHECK(in[8]== true);
    BOOST_CHECK(in[11] == true);
    BOOST_CHECK(in[12] == false);
    BOOST_CHECK_EQUAL(in.size(), writes);
}
