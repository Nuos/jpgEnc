#include "test/unittest.hpp"

#include "Image.hpp"
#include "BitstreamGeneric.hpp"
#include "JpegSegments.hpp"

BOOST_AUTO_TEST_CASE(image_loading_test) {
    auto image = loadPPM("res/tester_p3.ppm");
    BOOST_CHECK(image.R(0, 0) == 0);
    BOOST_CHECK(image.G(0, 0) == 0);
    BOOST_CHECK(image.B(0, 0) == 0);

    BOOST_CHECK(image.R(0, 3) == 255);
    BOOST_CHECK(image.G(0, 3) == 0);
    BOOST_CHECK(image.B(0, 3) == 255);

    BOOST_CHECK(image.R(2, 2) == 0);
    BOOST_CHECK(image.G(2, 2) == 255);
    BOOST_CHECK(image.B(2, 2) == 119);

    BOOST_CHECK(image.R(0, 0) == 0);
    BOOST_CHECK(image.G(0, 0) == 0);
    BOOST_CHECK(image.B(0, 0) == 0);

    // out of bounds indexing
    BOOST_CHECK(image.R(15, 0) == 255);
    BOOST_CHECK(image.G(15, 0) == 0);
    BOOST_CHECK(image.B(15, 0) == 255);

    BOOST_CHECK(image.R(0, 15) == 255);
    BOOST_CHECK(image.G(0, 15) == 0);
    BOOST_CHECK(image.B(0, 15) == 255);

    BOOST_CHECK(image.R(1, 15) == 0);
    BOOST_CHECK(image.G(1, 15) == 0);
    BOOST_CHECK(image.B(1, 15) == 0);

    BOOST_CHECK(image.R(15, 1) == 0);
    BOOST_CHECK(image.G(15, 1) == 0);
    BOOST_CHECK(image.B(15, 1) == 0);

    BOOST_CHECK(image.R(15, 15) == 0);
    BOOST_CHECK(image.G(15, 15) == 0);
    BOOST_CHECK(image.B(15, 15) == 0);
}

BOOST_AUTO_TEST_CASE(image_color_conv_test) {
    auto image = loadPPM("res/tester_p3.ppm");

    auto YCbCr_image = image.convertToColorSpace(Image::YCbCr);
    CHECK_CLOSE(YCbCr_image.Y(0, 3), -22.685);
    CHECK_CLOSE(YCbCr_image.Cb(0, 3), 84.4815);
    CHECK_CLOSE(YCbCr_image.Cr(0, 3), 106.7685);

    CHECK_CLOSE(YCbCr_image.Y(1, 1), 35.251);
    CHECK_CLOSE(YCbCr_image.Cb(1, 1), -24.956);
    CHECK_CLOSE(YCbCr_image.Cr(1, 1), -116.417698);

    // converting from YCbCr to YCbCr doesn't do a thing!
    YCbCr_image = image.convertToColorSpace(Image::YCbCr);
    CHECK_CLOSE(YCbCr_image.Y(0, 3), -22.685);
    CHECK_CLOSE(YCbCr_image.Cb(0, 3), 84.4815);
    CHECK_CLOSE(YCbCr_image.Cr(0, 3), 106.7685);

    auto rgb_image = image.convertToColorSpace(Image::RGB);
    CHECK_CLOSE(rgb_image.R(0, 3), 255);
    CHECK_CLOSE(rgb_image.G(0, 3), 0);
    CHECK_CLOSE(rgb_image.B(0, 3), 255);

    CHECK_CLOSE(rgb_image.R(1, 1), 0);
    CHECK_CLOSE(rgb_image.G(1, 1), 255);
    CHECK_CLOSE(rgb_image.B(1, 1), 119);
}

BOOST_AUTO_TEST_CASE(image_subsampling_test)
{
    auto image_orig = loadPPM("res/tester_p3.ppm");

    {
        auto image = image_orig;
        image.applySubsampling(Image::S444);
        BOOST_CHECK_EQUAL(image.B.size2(), 16);
        BOOST_CHECK_EQUAL(image.B.size1(), 16);
    }

    {
        auto image = image_orig;
        image.applySubsampling(Image::S422);
        BOOST_CHECK_EQUAL(image.B.size2(), 8);
        BOOST_CHECK_EQUAL(image.B.size1(), 16);

        // B channel:
        // 0 0 
        // 0 0
        // 0 7
        // 15 0
        BOOST_CHECK_EQUAL(image.B(2, 0), 0);
        BOOST_CHECK_EQUAL(image.B(2, 1), 119);
        BOOST_CHECK_EQUAL(image.B(3, 0), 255);
        BOOST_CHECK_EQUAL(image.B(3, 1), 0);

        // G channel:
        // 0 0 
        // 0 0
        // 0 15
        // 0 0
        BOOST_CHECK_EQUAL(image.G(2, 0), 0);
        BOOST_CHECK_EQUAL(image.G(2, 1), 255);
        BOOST_CHECK_EQUAL(image.G(3, 0), 0);
    }

    {
        auto image = image_orig;
        image.applySubsampling(Image::S411);
        BOOST_CHECK_EQUAL(image.B.size2(), 4);
        BOOST_CHECK_EQUAL(image.B.size1(), 16);

        // B channel:
        // 0
        // 0
        // 0
        // 15
        BOOST_CHECK_EQUAL(image.B(2, 0), 0);
        BOOST_CHECK_EQUAL(image.B(3, 0), 255);

        // G channel:
        // 0
        // 0
        // 0
        // 0
        BOOST_CHECK_EQUAL(image.G(2, 0), 0);
        BOOST_CHECK_EQUAL(image.G(3, 0), 0);
    }

    {
        auto image = image_orig;
        image.applySubsampling(Image::S420);
        BOOST_CHECK_EQUAL(image.B.size2(), 8);
        BOOST_CHECK_EQUAL(image.B.size1(), 8);
        
        // B channel:
        // 0 0
        // 0 7
        BOOST_CHECK_EQUAL(image.B(1, 0), 0);
        BOOST_CHECK_EQUAL(image.B(1, 1), 119);

        // G channel:
        // 0 0
        // 0 15
        BOOST_CHECK_EQUAL(image.G(1, 0), 0);
        BOOST_CHECK_EQUAL(image.G(1, 1), 255);
    }

    {
        auto image = image_orig;
        image.applySubsampling(Image::S420_m);
        BOOST_CHECK_EQUAL(image.B.size2(), 8);
        BOOST_CHECK_EQUAL(image.B.size1(), 8);
        
        // B channel:
        // 1.75 3.75
        // 3.75 1.75
        BOOST_CHECK_EQUAL(image.B(0, 0), 29.75);
        BOOST_CHECK_EQUAL(image.B(1, 0), 63.75);
        BOOST_CHECK_EQUAL(image.B(0, 1), 63.75);
        BOOST_CHECK_EQUAL(image.B(1, 1), 29.75);

        // G channel:
        // 3.75 0
        // 0    3.75
        BOOST_CHECK_EQUAL(image.G(0, 0), 63.75);
        BOOST_CHECK_EQUAL(image.G(1, 0), 0);
        BOOST_CHECK_EQUAL(image.G(0, 1), 0);
        BOOST_CHECK_EQUAL(image.G(1, 1), 63.75);
    }

    {
        auto image = image_orig;
        image.applySubsampling(Image::S420_lm);
        BOOST_CHECK_EQUAL(image.B.size2(), 8);
        BOOST_CHECK_EQUAL(image.B.size1(), 8);
        
        // B channel:
        // 0   0
        // 7.5 3.5
        BOOST_CHECK_EQUAL(image.B(0, 0), 0);
        BOOST_CHECK_EQUAL(image.B(0, 1), 0);
        BOOST_CHECK_EQUAL(image.B(1, 0), 127.5);
        BOOST_CHECK_EQUAL(image.B(1, 1), 59.5);

        // G channel:
        // 0 0
        // 0 7.5
        BOOST_CHECK_EQUAL(image.G(0, 0), 0);
        BOOST_CHECK_EQUAL(image.G(1, 0), 0);
        BOOST_CHECK_EQUAL(image.G(0, 1), 0);
        BOOST_CHECK_EQUAL(image.G(1, 1), 127.5);
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

    // writing dummy image segments
    {
        // one color 8x8
        auto img = loadPPM("res/tester_black.ppm");
        img.writeJPEG("tester_black_8x8_own_encoder.jpg");

        img = loadPPM("res/tester_white.ppm");
        img.writeJPEG("tester_white_8x8_own_encoder.jpg");

        img = loadPPM("res/tester_red.ppm");
        img.writeJPEG("tester_red_8x8_own_encoder.jpg");

        img = loadPPM("res/tester_green.ppm");
        img.writeJPEG("tester_green_8x8_own_encoder.jpg");

        img = loadPPM("res/tester_blue.ppm");
        img.writeJPEG("tester_blue_8x8_own_encoder.jpg");

        // one color 4x4
        img = loadPPM("res/tester_black_4x4.ppm");
        img.writeJPEG("tester_black_4x4_own_encoder.jpg");

        img = loadPPM("res/tester_white_4x4.ppm");
        img.writeJPEG("tester_white_4x4_own_encoder.jpg");

        img = loadPPM("res/tester_red_4x4.ppm");
        img.writeJPEG("tester_red_4x4_own_encoder.jpg");

        img = loadPPM("res/tester_green_4x4.ppm");
        img.writeJPEG("tester_green_4x4_own_encoder.jpg");

        img = loadPPM("res/tester_blue_4x4.ppm");
        img.writeJPEG("tester_blue_4x4_own_encoder.jpg");

        // various
        img = loadPPM("res/tester_p3.ppm");
        img.writeJPEG("tester_p3_4x4_own_encoder.jpg");

        img = loadPPM("res/tester_p3_8x8.ppm");
        img.writeJPEG("tester_p3_8x8_own_encoder.jpg");

        img = loadPPM("res/tester_p3_12x8.ppm");
        img.writeJPEG("tester_p3_12x8_own_encoder.jpg");

        img = loadPPM("res/tester_green_12x8.ppm");
        img.writeJPEG("tester_green_12x8_own_encoder.jpg");

        img = loadPPM("res/tester_green_blue_12x8.ppm");
        img.writeJPEG("tester_green_blue_12x8_own_encoder.jpg");

        img = loadPPM("res/tester_green_blue_8x12.ppm");
        img.writeJPEG("tester_green_blue_8x12_own_encoder.jpg");

        img = loadPPM("res/tester_RGB_26x19.ppm");
        img.writeJPEG("tester_RGB_26x19_own_encoder.jpg");

        img = loadPPM("res/tester_white_gray_black_32x32.ppm");
        img.writeJPEG("tester_white_gray_black_32x32_own_encoder.jpg");

        img = loadPPM("res/tester_text_32x32.ppm");
        img.writeJPEG("tester_text_32x32_own_encoder.jpg");
    }

#if NDEBUG
    // writing draigoch jpeg segments
    {
        auto image = loadPPM("res/Draigoch_p6.ppm");
        image.writeJPEG("Draigoch.jpeg");
    }
#endif

    // setting segment data
    {
        using namespace Segment;

        // APP0 Seg
        BOOST_CHECK_EQUAL(18, sizeof(sAPP0));

        auto APP0 = sAPP0().setLen(256)
                           .setXdensity(1)
                           .setYdensity(1);

        BOOST_CHECK_EQUAL(APP0.len[0], 1);
        BOOST_CHECK_EQUAL(APP0.len[1], 0);

        BOOST_CHECK_EQUAL(APP0.x_density[0], 0);
        BOOST_CHECK_EQUAL(APP0.x_density[1], 1);

        BOOST_CHECK_EQUAL(APP0.y_density[0], 0);
        BOOST_CHECK_EQUAL(APP0.y_density[1], 1);


        // SOF0 seg (3 component version)
        BOOST_CHECK_EQUAL(19, sizeof(Segment::sSOF0));

        auto SOF0 = sSOF0().setImageSizeX(256)
                           .setImageSizeY(256)
                           .setComponentSetup(
        { ComponentSetup::Y, ComponentSetup::NoSubSampling, 0,
          ComponentSetup::Cb, ComponentSetup::Half, 1,
          ComponentSetup::Cr, ComponentSetup::Half, 2, }
        );

        BOOST_CHECK_EQUAL(SOF0.image_size_x[0], 1);
        BOOST_CHECK_EQUAL(SOF0.image_size_x[1], 0);

        BOOST_CHECK_EQUAL(SOF0.image_size_y[0], 1);
        BOOST_CHECK_EQUAL(SOF0.image_size_y[1], 0);

        BOOST_CHECK_EQUAL(SOF0.component_setup[0], ComponentSetup::Y);
        BOOST_CHECK_EQUAL(SOF0.component_setup[1], ComponentSetup::NoSubSampling);
        BOOST_CHECK_EQUAL(SOF0.component_setup[2], 0);
        BOOST_CHECK_EQUAL(SOF0.component_setup[3], ComponentSetup::Cb);
        BOOST_CHECK_EQUAL(SOF0.component_setup[4], ComponentSetup::Half);
        BOOST_CHECK_EQUAL(SOF0.component_setup[5], 1);
        BOOST_CHECK_EQUAL(SOF0.component_setup[6], ComponentSetup::Cr);
        BOOST_CHECK_EQUAL(SOF0.component_setup[7], ComponentSetup::Half);
        BOOST_CHECK_EQUAL(SOF0.component_setup[8], 2);
    }
}

BOOST_AUTO_TEST_CASE(applying_dct) {
    // applying dct
    {
        auto image = loadPPM("res/tester_p3.ppm");
        image.applyDCT(Image::Matrix);
    }
}