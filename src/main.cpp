#include <iostream>

#include <boost/dynamic_bitset.hpp>

#include "Image.hpp"

int main(int argc, char argv[]) {
    {
        // least significant bit = x[0]
        boost::dynamic_bitset<> x(5); // all 0's by default
        x[0] = 1;
        x[1] = 1;
        x[4] = 1;

        for (boost::dynamic_bitset<>::size_type i = 0; i < x.size(); ++i)
            std::cout << x[i];
        std::cout << "\n";

        std::cout << x << "\n";
    }

    auto image = loadPPM("test/tester_p3.ppm");
    assert(image.R.at(100, 0) == 15);
    assert(image.G.at(100, 0) == 0);
    assert(image.B.at(100, 0) == 15);
    
    auto YCbCr_image = image.convertToColorSpace(ColorSpace::YCbCr);
    assert(YCbCr_image. Y.at(3, 0) == 6);
    assert(YCbCr_image.Cb.at(3, 0) == 132);
    assert(YCbCr_image.Cr.at(3, 0) == 134);

    YCbCr_image = image.convertToColorSpace(ColorSpace::YCbCr);
    assert(YCbCr_image. Y.at(3, 0) == 6);
    assert(YCbCr_image.Cb.at(3, 0) == 132);
    assert(YCbCr_image.Cr.at(3, 0) == 134);

    auto rgb_image = image.convertToColorSpace(ColorSpace::RGB);
    assert(rgb_image.R.at(3, 0) == 15);
    assert(rgb_image.G.at(3, 0) == 0);
    assert(rgb_image.B.at(3, 0) == 15);

    return 0;
}