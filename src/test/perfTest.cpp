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

#define PRINT_TEST_NAME(TYPE) std::cout << "\n>>> " << __FUNCTION__ << ": " << typeid(TYPE).name() << "\n"

#define TIME(description, function) timeFn(#description, [&]() { #function });

void timeFn(std::string desc, std::function<void()> fn)
{
    auto start = high_resolution_clock::now();
    fn();
    auto end = high_resolution_clock::now();
    std::cout << desc << " took " << duration_cast<milliseconds>(end - start).count() << " ms\n";
}

template <typename TestType>
void test_bitstream()
{
    PRINT_TEST_NAME(TestType);

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

void test_jpeg_segment_writing()
{
    auto image = loadPPM("res/tester_p3.ppm");
    image.writeJPEG(L"tester_p3.jpg");
    //timeFn("Writing JPEG Segments", [&]() { image.writeJPEG(L"tester_p3.jpg"); });
}

int main()
{
    srand((unsigned int) time(NULL));

    test_bitstream<boost::dynamic_bitset<>>();
    test_bitstream<Bitstream64>();
    test_bitstream<Bitstream8>();

    timeFn("\nWriting JPEG Segments", &test_jpeg_segment_writing);
    //test_jpeg_segment_writing();

    return 0;
}