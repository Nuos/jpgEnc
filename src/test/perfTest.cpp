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
const auto writes = 1e5;
#else
const auto writes = 1e8;
#endif

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

void test_dct_arai() {
    PRINT_TEST_NAME;
    
    matrix<PixelDataType> m(8, 8);
    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            m(y, x) = (x + 8 * y) % 256;
        }
    }
    matrix<PixelDataType> dct;
    
#if _DEBUG
    const int count = 1e5;
#else
    const int count = 1e7;
#endif

    // first version
    auto start = high_resolution_clock::now();
    for (int i = 0; i < count; i++) {
        dct = dctArai2(m);
    }
    auto end = high_resolution_clock::now();
    std::cout << count << " 8x8 DCT with arai2: " << duration_cast<milliseconds>(end - start).count() << "ms";
    std::cout << " avg: " << duration_cast<milliseconds>(end - start).count() * 1.0 / count << "ms\n";


    // second version
    start = high_resolution_clock::now();
    for (int i = 0; i < count; i++) {
        dct = dctArai(m);
    }
    end = high_resolution_clock::now();
    std::cout << count << " 8x8 DCT with arai: " << duration_cast<milliseconds>(end - start).count() << "ms";
    std::cout << " avg: " << duration_cast<milliseconds>(end - start).count() * 1.0 / count << "ms\n";
}

void test_dct_matrix() {
    PRINT_TEST_NAME;
    
    matrix<PixelDataType> m(8, 8);
    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            m(y, x) = (x + 8 * y) % 256;
        }
    }
    matrix<PixelDataType> dct;
    
#if _DEBUG
    const int count = 1e2;
#else
    const int count = 2e6;
#endif

    // matrix version
    auto start = high_resolution_clock::now();
    for (int i = 0; i < count; i++) {
        dct = dctMat(m);
    }
    auto end = high_resolution_clock::now();
    std::cout << count << " 8x8 DCT with matrix: " << duration_cast<milliseconds>(end - start).count() << "ms";
    std::cout << " avg: " << duration_cast<milliseconds>(end - start).count() * 1.0 / count << "ms\n";
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

// takes in release build alltogether around 20 seconds on a 3.0 GHz Intel Q9650  Processor
void test_dcts()
{
    PRINT_TEST_NAME;

    auto img = loadPerformanceTestImage();

#if _DEBUG
    const uint count = 1e1;
#else
    const uint count = 1e3;
#endif

    auto copy_img = img;
    long long duration = 0;

    auto LogOneTransformDuration = [count, &img](long long duration){
        printf("\tOne %dx%d image: %f ms\n", img.width, img.height, duration * 1.0 / count);
    };

    printf("Processing DCT %d times!\n", count);

    //// Ugh! SLOOOOOW!
    //duration = timeFn("Simple Dct", [&copy_img, count]() { 
    //    for (auto i = 0U; i < count; ++i)
    //        copy_img.applyDCT(Image::DCTMode::Simple);
    //});
    //LogOneTransformDuration(duration);

    duration = timeFn("Matrix Dct", [&copy_img, count]() { 
        for (auto i = 0U; i < count; ++i)
            copy_img.applyDCT(Image::DCTMode::Matrix);
    });
    LogOneTransformDuration(duration);

    duration = timeFn("Arai Dct", [&copy_img, count]() { 
        for (auto i = 0U; i < count; ++i)
            copy_img.applyDCT(Image::DCTMode::Arai);
    });
    LogOneTransformDuration(duration);

    duration = timeFn("Arai Fast Dct", [&copy_img, count]() { 
        for (auto i = 0U; i < count; ++i)
            copy_img.applyDCT(Image::DCTMode::Arai2Fast);
    });
    LogOneTransformDuration(duration);
}

int main()
{
    //test_bitstream<boost::dynamic_bitset<>>();
    //test_bitstream<Bitstream64>();
    //test_bitstream<Bitstream8>();

    //test_ppm_loading();
    //test_jpeg_segment_writing();

    //test_dct_arai();
    //test_dct_matrix();

    test_dcts();

    return 0;
}