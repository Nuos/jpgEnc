#include <iostream>
#include <chrono>
#include <fstream>
#include <functional>

#include <boost/dynamic_bitset.hpp>

#include "BitstreamGeneric.hpp"
#include "Image.hpp"

using namespace std::chrono;

#if _DEBUG
const auto writes = 1e5;
#else
const auto writes = 1e8;
#endif

#define PRINT_TEST_NAME std::cout << "\n>>> " << __FUNCTION__ << ":\n"
#define PRINT_TEST_NAME_TYPE(TYPE) std::cout << "\n>>> " << __FUNCTION__ << ": " << typeid(TYPE).name() << "\n"

#define TIME(description, function) timeFn(#description, [&]() { #function });

void timeFn(std::string desc, std::function<void()> fn)
{
    auto start = high_resolution_clock::now();
    fn();
    auto end = high_resolution_clock::now();
    std::cout << desc << " took " << duration_cast<milliseconds>(end - start).count() << " ms\n";
}

void timeFn(std::function<void()> fn) { timeFn("", fn); }

template <typename TestType>
void test_bitstream()
{
    PRINT_TEST_NAME_TYPE(TestType);

    TestType bitset;

    // appending bits in blocks of four (four 1s, four 0s, ...)
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
}

void test_ppm_loading() {
    PRINT_TEST_NAME;
    timeFn("loading p3 ppm", [&]() { loadPPM("res/tester_p3.ppm"); });
    timeFn("loading p6 ppm", [&]() { loadPPM("res/tester_p6.ppm"); });
    timeFn("loading draigoch p6 ppm", [&]() { loadPPM("res/Draigoch_p6.ppm"); });
    timeFn("loading draigoch p3 ppm", [&]() { loadPPM("res/Draigoch_p3.ppm"); });
}

void test_jpeg_segment_writing()
{
    PRINT_TEST_NAME;
    auto image = loadPPM("res/Draigoch_p6.ppm");
    timeFn("writing jpeg segments", [&]() { image.writeJPEG(L"Draigoch.jpg"); });
}

int main()
{
    test_bitstream<boost::dynamic_bitset<>>();
    test_bitstream<Bitstream64>();
    test_bitstream<Bitstream8>();

    test_ppm_loading();
    test_jpeg_segment_writing();

    return 0;
}