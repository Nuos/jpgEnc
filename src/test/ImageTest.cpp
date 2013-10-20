#include <boost/test/unit_test.hpp>

#include "Image.hpp"
#include "BitstreamGeneric.hpp"

int add(int i, int j) { return i+j; }

BOOST_AUTO_TEST_CASE(test_add) {
    // seven ways to detect and report the same error:
    BOOST_CHECK(add(2, 2) == 4);        // #1 continues on error

    BOOST_REQUIRE(add(2, 2) == 4);      // #2 throws on error

    if (add(2, 2) != 4)
        BOOST_ERROR("Ouch...");            // #3 continues on error

    if (add(2, 2) != 4)
        BOOST_FAIL("Ouch...");             // #4 throws on error

    if (add(2, 2) != 4) throw "Ouch..."; // #5 throws on error

    BOOST_CHECK_MESSAGE(add(2, 2) == 4,  // #6 continues on error
                        "add(..) result: " << add(2, 2));

    BOOST_CHECK_EQUAL(add(2, 2), 4);	  // #7 continues on error
}

BOOST_AUTO_TEST_CASE(image_color_conv_test) {
    auto image = loadPPM("res/tester_p3.ppm");
    BOOST_CHECK(image.R.at(100, 0) == 15);
    BOOST_CHECK(image.G.at(100, 0) == 0);
    BOOST_CHECK(image.B.at(100, 0) == 15);

    BOOST_CHECK(image.R.at(1, 1) == 0);
    BOOST_CHECK(image.G.at(1, 1) == 15);
    BOOST_CHECK(image.B.at(1, 1) == 7);

    auto YCbCr_image = image.convertToColorSpace(ColorSpace::YCbCr);
    BOOST_CHECK(YCbCr_image.Y.at(3, 0) == 6);
    BOOST_CHECK(YCbCr_image.Cb.at(3, 0) == 132);
    BOOST_CHECK(YCbCr_image.Cr.at(3, 0) == 134);

    YCbCr_image = image.convertToColorSpace(ColorSpace::YCbCr);
    BOOST_CHECK(YCbCr_image.Y.at(3, 0) == 6);
    BOOST_CHECK(YCbCr_image.Cb.at(3, 0) == 132);
    BOOST_CHECK(YCbCr_image.Cr.at(3, 0) == 134);

    auto rgb_image = image.convertToColorSpace(ColorSpace::RGB);
    BOOST_CHECK(rgb_image.R.at(3, 0) == 15);
    BOOST_CHECK(rgb_image.G.at(3, 0) == 0);
    BOOST_CHECK(rgb_image.B.at(3, 0) == 15);
}

BOOST_AUTO_TEST_CASE(image_subsampling_test)
{
    auto image_orig = loadPPM("res/tester_p3.ppm");

    {
        auto image = image_orig;
        image.applySubsampling(Image::S444);
        BOOST_CHECK_EQUAL(image.B.width, 4);
        BOOST_CHECK_EQUAL(image.B.height, 4);
    }

    {
        auto image = image_orig;
        image.applySubsampling(Image::S422);
        BOOST_CHECK_EQUAL(image.B.width, 2);
        BOOST_CHECK_EQUAL(image.B.height, 4);

        // B channel:
        // 0 0 
        // 0 0
        // 0 7
        // 15 0
        BOOST_CHECK_EQUAL(image.B.at(0, 2), 0);
        BOOST_CHECK_EQUAL(image.B.at(1, 2), 7);
        BOOST_CHECK_EQUAL(image.B.at(0, 3), 15);
        BOOST_CHECK_EQUAL(image.B.at(1, 3), 0);

        // G channel:
        // 0 0 
        // 0 0
        // 0 15
        // 0 0
        BOOST_CHECK_EQUAL(image.G.at(0, 2), 0);
        BOOST_CHECK_EQUAL(image.G.at(1, 2), 15);
        BOOST_CHECK_EQUAL(image.G.at(0, 3), 0);
    }

    {
        auto image = image_orig;
        image.applySubsampling(Image::S411);
        BOOST_CHECK_EQUAL(image.B.width, 1);
        BOOST_CHECK_EQUAL(image.B.height, 4);

        // B channel:
        // 0
        // 0
        // 0
        // 15
        BOOST_CHECK_EQUAL(image.B.at(0, 2), 0);
        BOOST_CHECK_EQUAL(image.B.at(0, 3), 15);

        // G channel:
        // 0
        // 0
        // 0
        // 0
        BOOST_CHECK_EQUAL(image.G.at(0, 2), 0);
        BOOST_CHECK_EQUAL(image.G.at(0, 3), 0);
    }

    {
        auto image = image_orig;
        image.applySubsampling(Image::S420);
        BOOST_CHECK_EQUAL(image.B.width, 2);
        BOOST_CHECK_EQUAL(image.B.height, 2);
        
        // B channel:
        // 0 0
        // 0 7
        BOOST_CHECK_EQUAL(image.B.at(0, 1), 0);
        BOOST_CHECK_EQUAL(image.B.at(1, 1), 7);

        // G channel:
        // 0 0
        // 0 15
        BOOST_CHECK_EQUAL(image.G.at(0, 1), 0);
        BOOST_CHECK_EQUAL(image.G.at(1, 1), 15);
    }

    {
        auto image = image_orig;
        image.applySubsampling(Image::S420_m);
        BOOST_CHECK_EQUAL(image.B.width, 2);
        BOOST_CHECK_EQUAL(image.B.height, 2);
        
        // B channel:
        // 1 3
        // 3 1
        BOOST_CHECK_EQUAL(image.B.at(0, 0), 1);
        BOOST_CHECK_EQUAL(image.B.at(0, 1), 3);
        BOOST_CHECK_EQUAL(image.B.at(1, 0), 3);
        BOOST_CHECK_EQUAL(image.B.at(1, 1), 1);

        // G channel:
        // 3 0
        // 0 3
        BOOST_CHECK_EQUAL(image.G.at(0, 0), 3);
        BOOST_CHECK_EQUAL(image.G.at(0, 1), 0);
        BOOST_CHECK_EQUAL(image.G.at(1, 0), 0);
        BOOST_CHECK_EQUAL(image.G.at(1, 1), 3);
    }

    {
        auto image = image_orig;
        image.applySubsampling(Image::S420_lm);
        BOOST_CHECK_EQUAL(image.B.width, 2);
        BOOST_CHECK_EQUAL(image.B.height, 2);
        
        // B channel:
        // 0 0
        // 7 3
        BOOST_CHECK_EQUAL(image.B.at(0, 0), 0);
        BOOST_CHECK_EQUAL(image.B.at(1, 0), 0);
        BOOST_CHECK_EQUAL(image.B.at(0, 1), 7);
        BOOST_CHECK_EQUAL(image.B.at(1, 1), 3);

        // G channel:
        // 0 0
        // 0 7
        BOOST_CHECK_EQUAL(image.G.at(0, 0), 0);
        BOOST_CHECK_EQUAL(image.G.at(1, 0), 0);
        BOOST_CHECK_EQUAL(image.G.at(0, 1), 0);
        BOOST_CHECK_EQUAL(image.G.at(1, 1), 7);
    }
}