#include <boost/test/unit_test.hpp>

#include "Image.hpp"
#include "BitstreamGeneric.hpp"
#include "JpegSegments.hpp"

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
    BOOST_CHECK(image.R(100, 0) == 15);
    BOOST_CHECK(image.G(100, 0) == 0);
    BOOST_CHECK(image.B(100, 0) == 15);

    BOOST_CHECK(image.R(1, 1) == 0);
    BOOST_CHECK(image.G(1, 1) == 15);
    BOOST_CHECK(image.B(1, 1) == 7);

    auto YCbCr_image = image.convertToColorSpace(Image::YCbCr);
    BOOST_CHECK(YCbCr_image.Y(3, 0) == 6);
    BOOST_CHECK(YCbCr_image.Cb(3, 0) == 133);
    BOOST_CHECK(YCbCr_image.Cr(3, 0) == 134);

    YCbCr_image = image.convertToColorSpace(Image::YCbCr);
    BOOST_CHECK(YCbCr_image.Y(3, 0) == 6);
    BOOST_CHECK(YCbCr_image.Cb(3, 0) == 133);
    BOOST_CHECK(YCbCr_image.Cr(3, 0) == 134);

    auto rgb_image = image.convertToColorSpace(Image::RGB);
    BOOST_CHECK(rgb_image.R(3, 0) == 15);
    BOOST_CHECK(rgb_image.G(3, 0) == 0);
    BOOST_CHECK(rgb_image.B(3, 0) == 15);
}

BOOST_AUTO_TEST_CASE(image_subsampling_test)
{
    auto image_orig = loadPPM("res/tester_p3.ppm");

    {
        auto image = image_orig;
        image.applySubsampling(Image::S444);
        BOOST_CHECK_EQUAL(image.B.w, 4);
        BOOST_CHECK_EQUAL(image.B.h, 4);
    }

    {
        auto image = image_orig;
        image.applySubsampling(Image::S422);
        BOOST_CHECK_EQUAL(image.B.w, 2);
        BOOST_CHECK_EQUAL(image.B.h, 4);

        // B channel:
        // 0 0 
        // 0 0
        // 0 7
        // 15 0
        BOOST_CHECK_EQUAL(image.B(0, 2), 0);
        BOOST_CHECK_EQUAL(image.B(1, 2), 7);
        BOOST_CHECK_EQUAL(image.B(0, 3), 15);
        BOOST_CHECK_EQUAL(image.B(1, 3), 0);

        // G channel:
        // 0 0 
        // 0 0
        // 0 15
        // 0 0
        BOOST_CHECK_EQUAL(image.G(0, 2), 0);
        BOOST_CHECK_EQUAL(image.G(1, 2), 15);
        BOOST_CHECK_EQUAL(image.G(0, 3), 0);
    }

    {
        auto image = image_orig;
        image.applySubsampling(Image::S411);
        BOOST_CHECK_EQUAL(image.B.w, 1);
        BOOST_CHECK_EQUAL(image.B.h, 4);

        // B channel:
        // 0
        // 0
        // 0
        // 15
        BOOST_CHECK_EQUAL(image.B(0, 2), 0);
        BOOST_CHECK_EQUAL(image.B(0, 3), 15);

        // G channel:
        // 0
        // 0
        // 0
        // 0
        BOOST_CHECK_EQUAL(image.G(0, 2), 0);
        BOOST_CHECK_EQUAL(image.G(0, 3), 0);
    }

    {
        auto image = image_orig;
        image.applySubsampling(Image::S420);
        BOOST_CHECK_EQUAL(image.B.w, 2);
        BOOST_CHECK_EQUAL(image.B.h, 2);
        
        // B channel:
        // 0 0
        // 0 7
        BOOST_CHECK_EQUAL(image.B(0, 1), 0);
        BOOST_CHECK_EQUAL(image.B(1, 1), 7);

        // G channel:
        // 0 0
        // 0 15
        BOOST_CHECK_EQUAL(image.G(0, 1), 0);
        BOOST_CHECK_EQUAL(image.G(1, 1), 15);
    }

    {
        auto image = image_orig;
        image.applySubsampling(Image::S420_m);
        BOOST_CHECK_EQUAL(image.B.w, 2);
        BOOST_CHECK_EQUAL(image.B.h, 2);
        
        // B channel:
        // 1 3
        // 3 1
        BOOST_CHECK_EQUAL(image.B(0, 0), 1);
        BOOST_CHECK_EQUAL(image.B(0, 1), 3);
        BOOST_CHECK_EQUAL(image.B(1, 0), 3);
        BOOST_CHECK_EQUAL(image.B(1, 1), 1);

        // G channel:
        // 3 0
        // 0 3
        BOOST_CHECK_EQUAL(image.G(0, 0), 3);
        BOOST_CHECK_EQUAL(image.G(0, 1), 0);
        BOOST_CHECK_EQUAL(image.G(1, 0), 0);
        BOOST_CHECK_EQUAL(image.G(1, 1), 3);
    }

    {
        auto image = image_orig;
        image.applySubsampling(Image::S420_lm);
        BOOST_CHECK_EQUAL(image.B.w, 2);
        BOOST_CHECK_EQUAL(image.B.h, 2);
        
        // B channel:
        // 0 0
        // 7 3
        BOOST_CHECK_EQUAL(image.B(0, 0), 0);
        BOOST_CHECK_EQUAL(image.B(1, 0), 0);
        BOOST_CHECK_EQUAL(image.B(0, 1), 7);
        BOOST_CHECK_EQUAL(image.B(1, 1), 3);

        // G channel:
        // 0 0
        // 0 7
        BOOST_CHECK_EQUAL(image.G(0, 0), 0);
        BOOST_CHECK_EQUAL(image.G(1, 0), 0);
        BOOST_CHECK_EQUAL(image.G(0, 1), 0);
        BOOST_CHECK_EQUAL(image.G(1, 1), 7);
    }
}

BOOST_AUTO_TEST_CASE(jpeg_segments_test)
{
    // playing with byte memory
    {
        Segment::Bytes<5> bytes{ { 0 } };
        Segment::set(bytes, { 1, 2, 3, 4, 255 });

        BOOST_CHECK_EQUAL(5, sizeof(bytes));

        BOOST_CHECK_EQUAL(bytes[0], 1);
        BOOST_CHECK_EQUAL(bytes[1], 2);
        BOOST_CHECK_EQUAL(bytes[2], 3);
        BOOST_CHECK_EQUAL(bytes[3], 4);
        BOOST_CHECK_EQUAL(bytes[4], 255);

        Byte copy[] = { 'J', 'F', 'I', 'F', '\0' };
        memcpy(bytes.data(), copy, 5);

        BOOST_CHECK_EQUAL(bytes[0], 'J');
        BOOST_CHECK_EQUAL(bytes[1], 'F');
        BOOST_CHECK_EQUAL(bytes[2], 'I');
        BOOST_CHECK_EQUAL(bytes[3], 'F');
        BOOST_CHECK_EQUAL(bytes[4], '\0');
    }

    // writing jpeg segments
    {
        Image img(4, 4, Image::RGB);
        img.writeJPEG(L"abc.jpeg");
    }

    // writing jpeg segments
    {
        auto image = loadPPM("res/Draigoch_p6.ppm");
        image.writeJPEG(L"Draigoch.jpeg");
    }

    // setting segment data
    {
        using namespace Segment;

        // APP0 Seg
        BOOST_CHECK_EQUAL(18, sizeof(APP0));

        APP0.setLen(256)
            .setXdensity(1)
            .setYdensity(1);

        BOOST_CHECK_EQUAL(APP0.len[0], 1);
        BOOST_CHECK_EQUAL(APP0.len[1], 0);

        BOOST_CHECK_EQUAL(APP0.x_density[0], 0);
        BOOST_CHECK_EQUAL(APP0.x_density[1], 1);

        BOOST_CHECK_EQUAL(APP0.y_density[0], 0);
        BOOST_CHECK_EQUAL(APP0.y_density[1], 1);


        // SOF0 seg (3 component version)
        BOOST_CHECK_EQUAL(13, sizeof(Segment::SOF0_1c));
        BOOST_CHECK_EQUAL(19, sizeof(Segment::SOF0_3c));

        SOF0_3c.setImageSizeX(256)
               .setImageSizeY(256)
               .setCompSetup(
        { CompSetup::Y, CompSetup::NoSubSampling, 0,
          CompSetup::Cb, CompSetup::Half, 1,
          CompSetup::Cr, CompSetup::Half, 2, }
        );

        BOOST_CHECK_EQUAL(SOF0_3c.image_size_x[0], 1);
        BOOST_CHECK_EQUAL(SOF0_3c.image_size_x[1], 0);

        BOOST_CHECK_EQUAL(SOF0_3c.image_size_y[0], 1);
        BOOST_CHECK_EQUAL(SOF0_3c.image_size_y[1], 0);

        BOOST_CHECK_EQUAL(SOF0_3c.component_setup[0], CompSetup::Y);
        BOOST_CHECK_EQUAL(SOF0_3c.component_setup[1], CompSetup::NoSubSampling);
        BOOST_CHECK_EQUAL(SOF0_3c.component_setup[2], 0);
        BOOST_CHECK_EQUAL(SOF0_3c.component_setup[3], CompSetup::Cb);
        BOOST_CHECK_EQUAL(SOF0_3c.component_setup[4], CompSetup::Half);
        BOOST_CHECK_EQUAL(SOF0_3c.component_setup[5], 1);
        BOOST_CHECK_EQUAL(SOF0_3c.component_setup[6], CompSetup::Cr);
        BOOST_CHECK_EQUAL(SOF0_3c.component_setup[7], CompSetup::Half);
        BOOST_CHECK_EQUAL(SOF0_3c.component_setup[8], 2);
    }
}