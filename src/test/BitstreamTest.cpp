#include <boost/test/unit_test.hpp>

#include <iostream>
#include <chrono>
#include <fstream>
#include <boost/dynamic_bitset.hpp>

BOOST_AUTO_TEST_CASE(test_dynamic_bitset) {
    using namespace std::chrono;

    boost::dynamic_bitset<> bitset;
    srand((unsigned int)time(NULL));

    bool val = false;
    auto start = high_resolution_clock::now();
    for (int x = 0; x < 1e6; ++x) {
        if (!(x%5)) val = !val;
        bitset.push_back(val);
    }
    auto end = high_resolution_clock::now();

    std::cout << "Adding a million bits: " << duration_cast<milliseconds>(end-start).count() << " ms\n";

    {
        std::ofstream bitset_out("saved_bitset", std::fstream::binary);
        bitset_out << bitset;
}

    boost::dynamic_bitset<> in;
    {
        std::ifstream bitset_in("saved_bitset");
        bitset_in >> in;
}

    BOOST_CHECK_EQUAL(in[0], true);
    BOOST_CHECK_EQUAL(in[4], true);
    BOOST_CHECK_EQUAL(in[5], false);
    BOOST_CHECK_EQUAL(in[9], false);
    BOOST_CHECK_EQUAL(in[10], true);
}
