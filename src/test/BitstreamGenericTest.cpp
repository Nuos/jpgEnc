#include <boost/test/unit_test.hpp>

#include <iostream>
#include <chrono>
#include <fstream>

#include "BitstreamGeneric.hpp"

#if _DEBUG
const auto writes = 1e4;
#else
const auto writes = 1e7;
#endif

BOOST_AUTO_TEST_CASE(test_own_bitstream_long)
{
    // PERF TESTS
    {
        std::cout << "\n>>> test_own_bitstream_long\n";
        using namespace std::chrono;

        // appending bits in blocks of four (four 1s, four 0s, ...)
        Bitstream bitset;
        srand((unsigned int) time(NULL));

        bool val = false;
        auto start = high_resolution_clock::now();
        for (int x = 0; x < writes; ++x) {
            if (!(x % 4)) val = !val;
            bitset << val;
        }
        auto end = high_resolution_clock::now();

        std::cout << "Adding " << writes << " bits: " << duration_cast<milliseconds>(end - start).count() << " ms\n";
        std::cout << "Bitset size: " << bitset.size() << " elements\n";

        start = high_resolution_clock::now();
        {
            // writing bitstream
            std::ofstream bitset_out("saved_bitset", std::fstream::binary);
            bitset_out << bitset;
        }

        Bitstream in;
        {
            // reading bitstream
            std::ifstream bitset_in("saved_bitset", std::fstream::binary);
            bitset_in >> in;
        }
        end = high_resolution_clock::now();

        std::cout << "Writing to and reading from file: " << duration_cast<milliseconds>(end - start).count() << " ms\n";

        // calculate real written bits to file as 64bit isn't aligned with 1e4 bits and we write only full blocks
        auto lwrites = (long long) writes;
        auto aligned = (0 == lwrites - ((lwrites / Bitstream::block_size) * Bitstream::block_size));
        auto real_writes = aligned ? lwrites : ((lwrites / Bitstream::block_size) * Bitstream::block_size + Bitstream::block_size);

        BOOST_CHECK(in[0]  == true);
        BOOST_CHECK(in[3]  == true);
        BOOST_CHECK(in[4]  == false);
        BOOST_CHECK(in[7]  == false);
        BOOST_CHECK(in[8]  == true);
        BOOST_CHECK(in[11] == true);
        BOOST_CHECK(in[12] == false);
        BOOST_CHECK_EQUAL(in.size(), writes);
    }

    // UNIT TESTS
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
        b0[8] = true;
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
    }

}
