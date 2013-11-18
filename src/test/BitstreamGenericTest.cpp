#include <boost/test/unit_test.hpp>

#include <iostream>
#include <chrono>
#include <fstream>

#include "BitstreamGeneric.hpp"

const auto writes = 1e3;

BOOST_AUTO_TEST_CASE(test_own_generic_bitstream)
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

    // appending arbitrary number of bits from an int
    auto b8 = Bitstream8();

    // 0011 0100 0000 0000 ...
    b8.push_back(0x34000000, 6);

    BOOST_CHECK(b8[0] == 0);
    BOOST_CHECK(b8[1] == 0);
    BOOST_CHECK(b8[2] == 1);
    BOOST_CHECK(b8[3] == 1);
    BOOST_CHECK(b8[4] == 0);
    BOOST_CHECK(b8[5] == 1);

    // extract arbitrary number of bits (up to 32 bits)
    b8 = Bitstream8{1,0,0,1,1,1};

    auto res = b8.extractT<uint16_t>(3, 0);
    BOOST_CHECK_EQUAL(res, 0x8000); // 10000..0

    res = b8.extractT<uint16_t>(4, 2);
    BOOST_CHECK_EQUAL(res, 0x7000); // 011100..0

    b8 = Bitstream8{ 1, 0, 0, 1, 1, 1, 0, 0,    1, 0, 1 };

    res = b8.extractT<uint16_t>(11, 0);
    BOOST_CHECK_EQUAL(res, 0x9CA0); // 1001110010100..0

    res = b8.extractT<uint16_t>(5, 6);
    BOOST_CHECK_EQUAL(res, 0x2800); // 0010100..0

    // extracting different types
    auto u8 = b8.extractT<uint8_t>(4, 1);
    BOOST_CHECK_EQUAL(u8, 0x30); // 00110000

    auto u16 = b8.extractT<uint16_t>(b8.size(), 0);
    BOOST_CHECK_EQUAL(u16, 0x9CA0); // 1001 1100 1010 0000

    auto u32 = b8.extractT<uint32_t>(5, 6);
    BOOST_CHECK_EQUAL(u32, 0x28000000); // 0010 100..0

    auto u64 = b8.extractT<uint64_t>(6, 3);
    BOOST_CHECK_EQUAL(u64, 0xE400000000000000); // 1110 0100 00..0

    // appending bitstreams
    auto b1 = Bitstream8{1, 0, 1, 1, 0, 0}; 
    auto b2 = Bitstream8{0, 0, 1, 1, 0, 0 };
    b1 << b2; // appending b2 to b1

    BOOST_CHECK_EQUAL(b1.size(), 12);
    BOOST_CHECK_EQUAL(b1.extractT<uint16_t>(b1.size(), 0), 0xB0C0);

    // writing / reading file
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

        // calculate real written bits to file as 64bit isn't aligned with 1e4 bits and we write only full blocks
        auto lwrites = (long long) writes;
        auto aligned = (0 == lwrites - ((lwrites / Bitstream::block_size) * Bitstream::block_size));
        auto real_writes = aligned ? lwrites : ((lwrites / Bitstream::block_size) * Bitstream::block_size + Bitstream::block_size);

        BOOST_CHECK_EQUAL(in[0], true);
        BOOST_CHECK_EQUAL(in[3], true);
        BOOST_CHECK_EQUAL(in[4], false);
        BOOST_CHECK_EQUAL(in[7], false);
        BOOST_CHECK_EQUAL(in[8], true);
        BOOST_CHECK_EQUAL(in[11], true);
        BOOST_CHECK_EQUAL(in[12], false);
        BOOST_CHECK_EQUAL(in.size(), real_writes);
    }

    // fill
    Bitstream8 bs{ 1, 0, 0, 1 };
    bs.fill();
    BOOST_CHECK_EQUAL(bs[0], true);
    BOOST_CHECK_EQUAL(bs[1], false);
    BOOST_CHECK_EQUAL(bs[2], false);
    BOOST_CHECK_EQUAL(bs[3], true);
    BOOST_CHECK_EQUAL(bs[4], true);
    BOOST_CHECK_EQUAL(bs[5], true);
    BOOST_CHECK_EQUAL(bs[6], true);
    BOOST_CHECK_EQUAL(bs[7], true);

    bs << Bits{ 0, 1 };
    BOOST_CHECK_EQUAL(bs[8], false);
    BOOST_CHECK_EQUAL(bs[9], true);

    // do nothing if the last block is already full
    Bitstream8 bs2{ 0, 0, 0, 0, 0, 0, 0, 0 };
    bs2.fill();
    BOOST_CHECK_EQUAL(bs2.size(), 8);

    // comparison operator
    Bitstream8 bs3{ 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1 };
    Bitstream8 bs4{ 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1 };
    BOOST_CHECK_EQUAL(bs3, bs4);

    Bitstream8 bs5{ 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 0 };
    BOOST_CHECK_NE(bs3, bs5);

    Bitstream8 bs6{ 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0 };
    BOOST_CHECK_NE(bs3, bs6);

}


BOOST_AUTO_TEST_CASE(extracting)
{
    // extract arbitrary number of bits (up to 32 bits)
    auto b = Bitstream64{ 1, 0, 0, 1, 1, 1 };

    auto res = b.extractT<uint16_t>(3, 0);
    BOOST_CHECK_EQUAL(res, 0x8000); // 10000..0

    res = b.extractT<uint16_t>(4, 2);
    BOOST_CHECK_EQUAL(res, 0x7000); // 011100..0

    b = Bitstream64{ 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 1 };

    res = b.extractT<uint16_t>(11, 0);
    BOOST_CHECK_EQUAL(res, 0x9CA0); // 1001110010100..0

    res = b.extractT<uint16_t>(5, 6);
    BOOST_CHECK_EQUAL(res, 0x2800); // 0010100..0


    // extracting different types
    auto u8 = b.extractT<uint8_t>(4, 1);
    BOOST_CHECK_EQUAL(u8, 0x30); // 00110000

    auto u16 = b.extractT<uint16_t>(b.size(), 0);
    BOOST_CHECK_EQUAL(u16, 0x9CA0); // 1001 1100 1010 0000

    auto u32 = b.extractT<uint32_t>(5, 6);
    BOOST_CHECK_EQUAL(u32, 0x28000000); // 0010 100..0

    auto u64 = b.extractT<uint64_t>(6, 3);
    BOOST_CHECK_EQUAL(u64, 0xE400000000000000); // 1110 0100 00..0
}