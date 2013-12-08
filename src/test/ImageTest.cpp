#include "test/unittest.hpp"

#include "Image.hpp"
#include "BitstreamGeneric.hpp"
#include "JpegSegments.hpp"

BOOST_AUTO_TEST_CASE(image_loading_test) {
    auto image = loadPPM("res/tester_p3.ppm");
    BOOST_CHECK(image.R(0, 0) == 0);
    BOOST_CHECK(image.G(0, 0) == 0);
    BOOST_CHECK(image.B(0, 0) == 0);

    BOOST_CHECK(image.R(0, 3) == 15);
    BOOST_CHECK(image.G(0, 3) == 0);
    BOOST_CHECK(image.B(0, 3) == 15);

    BOOST_CHECK(image.R(2, 2) == 0);
    BOOST_CHECK(image.G(2, 2) == 15);
    BOOST_CHECK(image.B(2, 2) == 7);

    BOOST_CHECK(image.R(0, 0) == 0);
    BOOST_CHECK(image.G(0, 0) == 0);
    BOOST_CHECK(image.B(0, 0) == 0);

    // out of bounds indexing
    BOOST_CHECK(image.R(7, 0) == 15);
    BOOST_CHECK(image.G(7, 0) == 0);
    BOOST_CHECK(image.B(7, 0) == 15);

    BOOST_CHECK(image.R(0, 7) == 15);
    BOOST_CHECK(image.G(0, 7) == 0);
    BOOST_CHECK(image.B(0, 7) == 15);

    BOOST_CHECK(image.R(1, 7) == 0);
    BOOST_CHECK(image.G(1, 7) == 0);
    BOOST_CHECK(image.B(1, 7) == 0);

    BOOST_CHECK(image.R(7, 1) == 0);
    BOOST_CHECK(image.G(7, 1) == 0);
    BOOST_CHECK(image.B(7, 1) == 0);

    BOOST_CHECK(image.R(7, 7) == 0);
    BOOST_CHECK(image.G(7, 7) == 0);
    BOOST_CHECK(image.B(7, 7) == 0);
}

BOOST_AUTO_TEST_CASE(image_color_conv_test) {
    auto image = loadPPM("res/tester_p3.ppm");

    auto YCbCr_image = image.convertToColorSpace(Image::YCbCr);
    CHECK_CLOSE(YCbCr_image.Y(0, 3), 6.195);
    CHECK_CLOSE(YCbCr_image.Cb(0, 3), 132.9695);
    CHECK_CLOSE(YCbCr_image.Cr(0, 3), 134.2805);

    CHECK_CLOSE(YCbCr_image. Y(1, 1), 9.6030);
    CHECK_CLOSE(YCbCr_image.Cb(1, 1), 126.532);
    CHECK_CLOSE(YCbCr_image.Cr(1, 1), 121.1519);

    // converting from YCbCr to YCbCr doesn't do a thing!
    YCbCr_image = image.convertToColorSpace(Image::YCbCr);
    CHECK_CLOSE(YCbCr_image.Y(0, 3), 6.195);
    CHECK_CLOSE(YCbCr_image.Cb(0, 3), 132.9695);
    CHECK_CLOSE(YCbCr_image.Cr(0, 3), 134.2805);

    auto rgb_image = image.convertToColorSpace(Image::RGB);
    CHECK_CLOSE(rgb_image.R(0, 3), 15);
    CHECK_CLOSE(rgb_image.G(0, 3), 0);
    CHECK_CLOSE(rgb_image.B(0, 3), 15);

    CHECK_CLOSE(rgb_image.R(1, 1), 0);
    CHECK_CLOSE(rgb_image.G(1, 1), 15);
    CHECK_CLOSE(rgb_image.B(1, 1), 7);
}

BOOST_AUTO_TEST_CASE(image_subsampling_test)
{
    auto image_orig = loadPPM("res/tester_p3.ppm");

    {
        auto image = image_orig;
        image.applySubsampling(Image::S444);
        BOOST_CHECK_EQUAL(image.B.size2(), 8);
        BOOST_CHECK_EQUAL(image.B.size1(), 8);
    }

    {
        auto image = image_orig;
        image.applySubsampling(Image::S422);
        BOOST_CHECK_EQUAL(image.B.size2(), 4);
        BOOST_CHECK_EQUAL(image.B.size1(), 8);

        // B channel:
        // 0 0 
        // 0 0
        // 0 7
        // 15 0
        BOOST_CHECK_EQUAL(image.B(2, 0), 0);
        BOOST_CHECK_EQUAL(image.B(2, 1), 7);
        BOOST_CHECK_EQUAL(image.B(3, 0), 15);
        BOOST_CHECK_EQUAL(image.B(3, 1), 0);

        // G channel:
        // 0 0 
        // 0 0
        // 0 15
        // 0 0
        BOOST_CHECK_EQUAL(image.G(2, 0), 0);
        BOOST_CHECK_EQUAL(image.G(2, 1), 15);
        BOOST_CHECK_EQUAL(image.G(3, 0), 0);
    }

    {
        auto image = image_orig;
        image.applySubsampling(Image::S411);
        BOOST_CHECK_EQUAL(image.B.size2(), 2);
        BOOST_CHECK_EQUAL(image.B.size1(), 8);

        // B channel:
        // 0
        // 0
        // 0
        // 15
        BOOST_CHECK_EQUAL(image.B(2, 0), 0);
        BOOST_CHECK_EQUAL(image.B(3, 0), 15);

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
        BOOST_CHECK_EQUAL(image.B.size2(), 4);
        BOOST_CHECK_EQUAL(image.B.size1(), 4);
        
        // B channel:
        // 0 0
        // 0 7
        BOOST_CHECK_EQUAL(image.B(1, 0), 0);
        BOOST_CHECK_EQUAL(image.B(1, 1), 7);

        // G channel:
        // 0 0
        // 0 15
        BOOST_CHECK_EQUAL(image.G(1, 0), 0);
        BOOST_CHECK_EQUAL(image.G(1, 1), 15);
    }

    {
        auto image = image_orig;
        image.applySubsampling(Image::S420_m);
        BOOST_CHECK_EQUAL(image.B.size2(), 4);
        BOOST_CHECK_EQUAL(image.B.size1(), 4);
        
        // B channel:
        // 1.75 3.75
        // 3.75 1.75
        BOOST_CHECK_EQUAL(image.B(0, 0), 1.75);
        BOOST_CHECK_EQUAL(image.B(1, 0), 3.75);
        BOOST_CHECK_EQUAL(image.B(0, 1), 3.75);
        BOOST_CHECK_EQUAL(image.B(1, 1), 1.75);

        // G channel:
        // 3.75 0
        // 0    3.75
        BOOST_CHECK_EQUAL(image.G(0, 0), 3.75);
        BOOST_CHECK_EQUAL(image.G(1, 0), 0);
        BOOST_CHECK_EQUAL(image.G(0, 1), 0);
        BOOST_CHECK_EQUAL(image.G(1, 1), 3.75);
    }

    {
        auto image = image_orig;
        image.applySubsampling(Image::S420_lm);
        BOOST_CHECK_EQUAL(image.B.size2(), 4);
        BOOST_CHECK_EQUAL(image.B.size1(), 4);
        
        // B channel:
        // 0   0
        // 7.5 3.5
        BOOST_CHECK_EQUAL(image.B(0, 0), 0);
        BOOST_CHECK_EQUAL(image.B(0, 1), 0);
        BOOST_CHECK_EQUAL(image.B(1, 0), 7.5);
        BOOST_CHECK_EQUAL(image.B(1, 1), 3.5);

        // G channel:
        // 0 0
        // 0 7.5
        BOOST_CHECK_EQUAL(image.G(0, 0), 0);
        BOOST_CHECK_EQUAL(image.G(1, 0), 0);
        BOOST_CHECK_EQUAL(image.G(0, 1), 0);
        BOOST_CHECK_EQUAL(image.G(1, 1), 7.5);
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
        Image img(4, 4, Image::RGB);
        img.writeJPEG(L"abc.jpeg");
    }

    // writing draigoch jpeg segments
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

BOOST_AUTO_TEST_CASE(applying_dct) {
    // applying dct
    {
        auto image = loadPPM("res/tester_p3.ppm");
        image.applyDCT(Image::Matrix);
    }
}