#define BOOST_TEST_MODULE ExampleTest
#include <boost/test/unit_test.hpp>

#include "Image.hpp"

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

BOOST_AUTO_TEST_CASE(image_test) {
    auto image = loadPPM("res/tester_p3.ppm");
    assert(image.r.at(100, 0) == 14);
    assert(image.g.at(100, 0) == 0);
    assert(image.b.at(100, 0) == 15);

    auto YCbCr_image = image.convertToColorSpace(ColorSpace::YCbCr);
    assert(YCbCr_image.y.at(3, 0) == 6);
    assert(YCbCr_image.cb.at(3, 0) == 132);
    assert(YCbCr_image.cr.at(3, 0) == 134);

    YCbCr_image = image.convertToColorSpace(ColorSpace::YCbCr);
    assert(YCbCr_image.y.at(3, 0) == 6);
    assert(YCbCr_image.cb.at(3, 0) == 132);
    assert(YCbCr_image.cr.at(3, 0) == 134);

    auto rgb_image = image.convertToColorSpace(ColorSpace::RGB);
    assert(rgb_image.r.at(3, 0) == 15);
    assert(rgb_image.g.at(3, 0) == 0);
    assert(rgb_image.b.at(3, 0) == 15);
}