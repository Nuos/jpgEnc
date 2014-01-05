#include "test/unittest.hpp"

#include "Coding.hpp"

BOOST_AUTO_TEST_CASE(rle_AC_test) {
    std::vector<int> data{ -111, 57, 0, 0, 0, 0, 0, 0,
                              0,  0, 0, 0, 0, 0, 0, 0,
                              0,  0, 0, 0, 3, 0, 0, 0,
                              0, -2, 0, 0, 0, 0, 0, 0, // -2 war ende vorher
                              0,  0, 0, 0, 0, 0, 0, 0,
                              0,  0, 0, 0, 0, 0, 0, 0,
                              0,  0, 0, 0, 0, 0, 0, 0,
                              0,  0, 0, 0, 0, 0, 0, -2
    }; 

    // -111 57 18*0 3 4*0 -2 37*0 -2
    std::vector<RLE_PAIR> expected_rle_data{
        RLE_PAIR(0, -111),
        RLE_PAIR(0, 57),
        RLE_PAIR(15, 0),
        RLE_PAIR(2, 3),
        RLE_PAIR(4, -2),
        RLE_PAIR(15, 0),
        RLE_PAIR(15, 0),
        RLE_PAIR(5, -2)
    };

    std::vector<int> zero_end_data{ -111, 57, 0, 0, 0, 0, 0, 0,
                                     0, 0, 0, 0, 0, 0, 0, 0,
                                     0, 0, 0, 0, 3, 0, 0, 0,
                                     0, -2, 0, 0, 0, 0, 0, 0,
                                     0, 0, 0, 0, 0, 0, 0, 0,
                                     0, 0, 0, 0, 0, 0, 0, 0,
                                     0, 0, 0, 0, 0, 0, 0, 0,
                                     0, 0, 0, 0, 0, 0, 0, 0
    };

    // -111 57 18*0 3 4*0 -2 x*0
    std::vector<RLE_PAIR> expected_rle_zero_end_data{
        RLE_PAIR(0, -111),
        RLE_PAIR(0, 57),
        RLE_PAIR(15, 0),
        RLE_PAIR(2, 3),
        RLE_PAIR(4, -2),
        RLE_PAIR(0, 0)
    };

    auto rle_data = RLE_AC(data);
    auto rle_zero_end_data = RLE_AC(zero_end_data);

    BOOST_CHECK(expected_rle_data == rle_data);
    BOOST_CHECK(expected_rle_zero_end_data == rle_zero_end_data);

    auto encoded_AC_coeffs = encode_category(rle_data);

    // got leaks when using initializer lists for vector initialization, so back to good old emplace/push_back
    std::vector<Category_Code> expected_coding;
    expected_coding.emplace_back(7, Bitstream(16, 7));
    expected_coding.emplace_back(6, Bitstream(57, 6));
    expected_coding.emplace_back(240, Bitstream());
    expected_coding.emplace_back(34, Bitstream(3, 2));
    expected_coding.emplace_back(66, Bitstream(1, 2));
    expected_coding.emplace_back(240, Bitstream());
    expected_coding.emplace_back(240, Bitstream());
    expected_coding.emplace_back(82, Bitstream(1, 2));

    BOOST_CHECK(expected_coding == encoded_AC_coeffs);
}

BOOST_AUTO_TEST_CASE(rle_AC_test_that_includes_zigzag) {
    std::vector<int> data{ -111, 57, 0, 0, 0, 0, 0, 0,
                              0,  0, 0, 0, 0, 0, 0, 0,
                              0,  0, 0, 0, 3, 0, 0, 0,
                              0, -2, 0, 0, 0, 0, 0, 0,
                              0,  0, 0, 0, 0, 0, 0, 0,
                              0,  0, 0, 0, 0, 0, 0, 0,
                              0,  0, 0, 0, 0, 0, 0, 0,
                              0,  0, 0, 0, 0, 0, 0, -2
    };
    auto mdata = from_vector(data);

    // zigzag: -111 57 9*0 -2 13*0 3 37*0 -2
    std::vector<RLE_PAIR> mexpected_rle_data{
        RLE_PAIR(0, -111),
        RLE_PAIR(0, 57),
        RLE_PAIR(9, -2),
        RLE_PAIR(13, 3),
        RLE_PAIR(15, 0),
        RLE_PAIR(15, 0),
        RLE_PAIR(5, -2)
    };

    std::vector<int> zero_end_data{ -111, 57, 0, 0, 0, 0, 0, 0,
                                       0,  0, 0, 0, 0, 0, 0, 0,
                                       0,  0, 0, 0, 3, 0, 0, 0,
                                       0, -2, 0, 0, 0, 0, 0, 0,
                                       0,  0, 0, 0, 0, 0, 0, 0,
                                       0,  0, 0, 0, 0, 0, 0, 0,
                                       0,  0, 0, 0, 0, 0, 0, 0,
                                       0,  0, 0, 0, 0, 0, 0, 0
    };
    auto mzero_end_data = from_vector(zero_end_data);

    // zigzag: -111 57 9*0 -2 13*0 3 38*0
    std::vector<RLE_PAIR> mexpected_rle_zero_end_data{
        RLE_PAIR(0, -111),
        RLE_PAIR(0, 57),
        RLE_PAIR(9, -2),
        RLE_PAIR(13, 3),
        RLE_PAIR(0, 0)
    };

    // matrix version
    auto mrle_data = RLE_AC(mdata);
    auto mrle_zero_end_data = RLE_AC(mzero_end_data);

    BOOST_CHECK(mexpected_rle_data == mrle_data);
    BOOST_CHECK(mexpected_rle_zero_end_data == mrle_zero_end_data);

    //auto encoded_AC_coeffs = encode_category(rle_data);

    //// got leaks when using initializer lists for vector initialization, so back to good old emplace/push_back
    //std::vector<Category_Code> expected_coding;
    //expected_coding.emplace_back(7, Bitstream(16, 7));
    //expected_coding.emplace_back(6, Bitstream(57, 6));
    //expected_coding.emplace_back(240, Bitstream());
    //expected_coding.emplace_back(34, Bitstream(3, 2));
    //expected_coding.emplace_back(66, Bitstream(1, 2));

    //BOOST_CHECK(expected_coding == encoded_AC_coeffs);
}

BOOST_AUTO_TEST_CASE(getCategoryAndCode_test) {
    short cat = 0;
    BOOST_CHECK(std::make_pair(cat, Bitstream()) == getCategoryAndCode(0));

    cat = 1;
    BOOST_CHECK(std::make_pair(cat, Bitstream(0, cat)) == getCategoryAndCode(-1));
    BOOST_CHECK(std::make_pair(cat, Bitstream(1, cat)) == getCategoryAndCode( 1));
                                                       
    cat = 2;                                           
    BOOST_CHECK(std::make_pair(cat, Bitstream(0, cat)) == getCategoryAndCode(-3));
    BOOST_CHECK(std::make_pair(cat, Bitstream(1, cat)) == getCategoryAndCode(-2));
    BOOST_CHECK(std::make_pair(cat, Bitstream(2, cat)) == getCategoryAndCode( 2));
    BOOST_CHECK(std::make_pair(cat, Bitstream(3, cat)) == getCategoryAndCode( 3));
                                                       
    cat = 3;                                           
    BOOST_CHECK(std::make_pair(cat, Bitstream(0, cat)) == getCategoryAndCode(-7));
    BOOST_CHECK(std::make_pair(cat, Bitstream(1, cat)) == getCategoryAndCode(-6));
    BOOST_CHECK(std::make_pair(cat, Bitstream(3, cat)) == getCategoryAndCode(-4));
    BOOST_CHECK(std::make_pair(cat, Bitstream(4, cat)) == getCategoryAndCode( 4));
    BOOST_CHECK(std::make_pair(cat, Bitstream(6, cat)) == getCategoryAndCode( 6));
    BOOST_CHECK(std::make_pair(cat, Bitstream(7, cat)) == getCategoryAndCode( 7));

    cat = 10;
    BOOST_CHECK(std::make_pair(cat, Bitstream(0, cat)) == getCategoryAndCode(-1023));
    BOOST_CHECK(std::make_pair(cat, Bitstream(1, cat)) == getCategoryAndCode(-1022));
    BOOST_CHECK(std::make_pair(cat, Bitstream(511, cat)) == getCategoryAndCode(-512));
    BOOST_CHECK(std::make_pair(cat, Bitstream(512, cat)) == getCategoryAndCode( 512));
    BOOST_CHECK(std::make_pair(cat, Bitstream(1022, cat)) == getCategoryAndCode( 1022));
    BOOST_CHECK(std::make_pair(cat, Bitstream(1023, cat)) == getCategoryAndCode( 1023));
}

