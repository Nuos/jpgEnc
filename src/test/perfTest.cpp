#include <iostream>
#include <chrono>
#include <fstream>
#include <functional>

#include <boost/dynamic_bitset.hpp>

#include "BitstreamGeneric.hpp"
#include "Image.hpp"
#include "Dct.hpp"

using namespace std::chrono;

#if _DEBUG
//#error Go into Release mode!
#endif

const auto writes = 1e8;

#define PRINT_TEST_NAME std::cout << "\n>>> " << __FUNCTION__ << ":\n"
#define PRINT_TEST_NAME_TYPE(TYPE) std::cout << "\n>>> " << __FUNCTION__ << ": " << typeid(TYPE).name() << "\n"

#define TIME(description, function) timeFn(#description, [&]() { #function });

long long timeFn(std::string desc, std::function<void()> fn)
{
    auto start = high_resolution_clock::now();
    fn();
    auto end = high_resolution_clock::now();
    std::cout << desc << " took " << duration_cast<milliseconds>(end - start).count() << " ms\n";
    return duration_cast<milliseconds>(end - start).count();
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

// generate image used for performance tests in assignment 4.4c
// Image is in Colorspace YBcCr and only channel Cb is used
Image loadPerformanceTestImage() {
    Image img(256, 256, Image::YCbCr);

    for (auto x = 0U; x < img.width; ++x) {
        for (auto y = 0U; y < img.height; ++y) {
            img.Cb(y, x) = (x + y*8) % 256;
        }
    }

    return img;
}

void test_dcts(float stretch_factor)
{
    PRINT_TEST_NAME;

    auto img = loadPerformanceTestImage();

    uint count = 0;

    auto copy_img = img;
    long long duration = 0;

    auto LogOneTransformDuration = [&img](long long duration, uint count){
        printf("\tOne %dx%d image: %f ms\n", img.width, img.height, duration * 1.0 / count);
    };

    count = 5000 * stretch_factor;
#if _DEBUG
    count /= 30;
#endif
    duration = timeFn(std::string("Simple Dct ") + std::to_string(count) + " times", [&copy_img, count]() {
        for (auto i = 0U; i < count; ++i)
            copy_img.applyDCT(Image::DCTMode::Simple);
    });
    LogOneTransformDuration(duration, count);

    count = 10000 * stretch_factor;
#if _DEBUG
    count /= 1000;
#endif
    duration = timeFn(std::string("Matrix Dct ") + std::to_string(count) + " times", [&copy_img, count]() {
        for (auto i = 0U; i < count; ++i)
            copy_img.applyDCT(Image::DCTMode::Matrix);
    });
    LogOneTransformDuration(duration, count);

    count = 30000 * stretch_factor;
#if _DEBUG
    count /= 200;
#endif
    duration = timeFn(std::string("Arai Fast Dct ") + std::to_string(count) + " times", [&copy_img, count]() {
        for (auto i = 0U; i < count; ++i)
            copy_img.applyDCT(Image::DCTMode::Arai);
    });
    LogOneTransformDuration(duration, count);
}

void test_encode_draigoch() {
    PRINT_TEST_NAME;

    auto image = loadPPM("res/Draigoch_p6.ppm");
    timeFn([&]() { image.writeJPEG(L"Draigoch.jpeg"); });
}

int main(int argc, const char** argv)
{
    //test_bitstream<boost::dynamic_bitset<>>();
    //test_bitstream<Bitstream64>();
    //test_bitstream<Bitstream8>();

    //test_ppm_loading();
    //test_jpeg_segment_writing();

    //test_dct_arai();
    //test_dct_matrix();

    float stretch_factor = 1.f;
    if (argc == 2)
        stretch_factor = std::stof(argv[1]);

    //test_dcts(stretch_factor);
    test_encode_draigoch();

    return 0;
}