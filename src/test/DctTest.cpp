#include "test/unittest.hpp"

#include <boost/numeric/ublas/io.hpp>
#include "Dct.hpp"

using mat = matrix<PixelDataType>;
using mat_byte = matrix<int8_t>;

std::vector<PixelDataType> zigzag(mat m) {
    std::vector<PixelDataType> r;
    const auto sz = m.size1() * m.size2();
    r.reserve(sz);

    size_t x = 0, y = 0;
    size_t dx = 1, dy = 0;
    int diag_dx = 1, diag_dy = -1;

    auto eof = &y;
    auto eof_t = &x;

    while (r.size() != sz) {
        while (*eof != 0 && *eof_t != 7) {
            r.push_back(m(y, x));
            x += diag_dx; y += diag_dy;
        }

        r.push_back(m(y, x));

        if (x + dx > 7 || y + dy > 7)
            std::swap(dx, dy);
        x += dx; y += dy;
        std::swap(dx, dy);
        std::swap(diag_dx, diag_dy);
        std::swap(eof, eof_t);
    }

    return r;
};

mat_byte quantize(const mat& m, const mat& table) {
    assert(m.size1() == 8);
    assert(m.size2() == 8);
    assert(table.size1() == 8);
    assert(table.size2() == 8);

    mat_byte result(8, 8);

    for (uint i = 0; i < 64; ++i) {
        result.data()[i] = static_cast<int8_t>(std::round(m.data()[i] / table.data()[i]));
    }

    return result;
}

mat from_vector(const std::vector<PixelDataType>& v) {
    assert(v.size() == 64);

    mat m(8, 8);
    for (size_t i = 0; i < m.size1(); i++) {
        for (size_t j = 0; j < m.size2(); j++) {
            m(i, j) = v[i*m.size2() + j];
        }
    }
    return m;
}

BOOST_AUTO_TEST_CASE(dct) {
    auto m = from_vector({
         1, 2,  3,  4,  5,  6,  7,  8,
         9, 10, 11, 12, 13, 14, 15, 16,
        17, 18, 19, 20, 21, 22, 23, 24,
        25, 26, 27, 28, 29, 30, 31, 32,
        33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48,
        49, 50, 51, 52, 53, 54, 55, 56,
        57, 58, 59, 60, 61, 62, 63, 64
    });
    BOOST_CHECK_EQUAL(m(2, 3), 20);

    auto true_dct = from_vector({
        260,               -18.2216411837961, 7.69085915161152e-15, -1.90481782616726, 0, -0.568239222367164, 1.85673764701218e-14, -0.143407824981022,
        -145.773129470369, 0,                 0, 0, 0, 0, 0, 0,
        0,                 0,                 0, 0, 0, 0, 0, 0,
        -15.2385426093380, 0,                 0, 0, 0, 0, 0, 0,
        0,                 0,                 0, 0, 0, 0, 0, 0,
        -4.54591377893732, 0,                 0, 0, 0, 0, 0, 0,
        0,                 0,                 0, 0, 0, 0, 0, 0,
        -1.14726259984816, 0,                 0, 0, 0, 0, 0, 0
    });

    matrix_range<mat> m_slice(m, range(0, 8), range(0, 8));

    mat dct(8, 8);
    matrix_range<mat> dct_slice(dct, range(0, 8), range(0, 8));

    dctArai(m_slice, dct_slice);
    CHECK_EQUAL_MAT(dct, true_dct);

    dct_slice *= 0;
    dctDirect(m_slice, dct_slice);
    CHECK_EQUAL_MAT(dct_slice, true_dct);

    dct_slice *= 0;
    dctMat(m_slice, dct_slice);
    CHECK_EQUAL_MAT(dct_slice, true_dct);
}

BOOST_AUTO_TEST_CASE(dct_matrix) {
    auto m = from_vector({
        1, 2, 3, 4, 5, 6, 7, 8,
        9, 10, 11, 12, 13, 14, 15, 16,
        17, 18, 19, 20, 21, 22, 23, 24,
        25, 26, 27, 28, 29, 30, 31, 32,
        33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48,
        49, 50, 51, 52, 53, 54, 55, 56,
        57, 58, 59, 60, 61, 62, 63, 64
    });
    BOOST_CHECK_EQUAL(m(2, 3), 20);

    auto true_dct = from_vector({
        260,               -18.2216411837961, 7.69085915161152e-15, -1.90481782616726, 0, -0.568239222367164, 1.85673764701218e-14, -0.143407824981022,
        -145.773129470369, 0,                 0, 0, 0, 0, 0, 0,
        0,                 0,                 0, 0, 0, 0, 0, 0,
        -15.2385426093380, 0,                 0, 0, 0, 0, 0, 0,
        0,                 0,                 0, 0, 0, 0, 0, 0,
        -4.54591377893732, 0,                 0, 0, 0, 0, 0, 0,
        0,                 0,                 0, 0, 0, 0, 0, 0,
        -1.14726259984816, 0,                 0, 0, 0, 0, 0, 0
    });

    matrix_range<mat> m_slice(m, range(0, 8), range(0, 8));

    mat dct(8, 8);
    matrix_range<mat> dct_slice(dct, range(0, 8), range(0, 8));

    dctMat(m_slice, dct_slice);
    CHECK_EQUAL_MAT(dct, true_dct);

    auto original = inverseDctMat(dct);
    CHECK_EQUAL_MAT(original, m);
}

BOOST_AUTO_TEST_CASE(zigzag_test) {
    auto m = from_vector({
        1, 2, 3, 4, 5, 6, 7, 8,
        9, 10, 11, 12, 13, 14, 15, 16,
        17, 18, 19, 20, 21, 22, 23, 24,
        25, 26, 27, 28, 29, 30, 31, 32,
        33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48,
        49, 50, 51, 52, 53, 54, 55, 56,
        57, 58, 59, 60, 61, 62, 63, 64
    });

    auto v = zigzag(m);

    //for (const auto& d : v)
    //    std::cout << d << ", ";

    auto exp = std::vector<PixelDataType>{
        1, 2, 9, 17, 10, 3, 4, 11, 18, 25, 33, 26, 19, 12, 5, 6, 13, 20, 27, 34, 41, 49, 42, 35, 28, 21, 14, 7, 8, 15, 22, 29,
        36, 43, 50, 57, 58, 51, 44, 37, 30, 23, 16, 24, 31, 38, 45, 52, 59, 60, 53, 46, 39, 32, 40, 47, 54, 61, 62, 55, 48, 56,
        63, 64};

    BOOST_CHECK_EQUAL_COLLECTIONS(begin(exp), end(exp), begin(v), end(v));
}


BOOST_AUTO_TEST_CASE(quantization) {
    const auto y_table = from_vector({
        16, 11, 10, 16,  24,  40,  51,  61,
        12, 12, 14, 19,  26,  58,  60,  55,
        14, 13, 16, 24,  40,  57,  69,  56,
        14, 17, 22, 29,  51,  87,  80,  62,
        18, 22, 37, 56,  68, 109, 103,  77,
        24, 35, 55, 64,  81, 104, 113,  92,
        49, 64, 78, 87, 103, 121, 120, 101,
        72, 92, 95, 98, 112, 100, 103,  99
    });
    const auto c_table = from_vector({
        17, 18, 24, 47, 99, 99, 99, 99,
        18, 21, 26, 66, 99, 99, 99, 99,
        24, 26, 56, 99, 99, 99, 99, 99,
        47, 66, 99, 99, 99, 99, 99, 99,
        99, 99, 99, 99, 99, 99, 99, 99,
        99, 99, 99, 99, 99, 99, 99, 99,
        99, 99, 99, 99, 99, 99, 99, 99,
        99, 99, 99, 99, 99, 99, 99, 99
    });

    auto input = from_vector({
        581, -144, 56, 17, 15, -7, 25, -9,
        -242, 133, -48, 42, -2, -7, 13, -4,
        108, -18, -40, 71, -33, 12, 6, -10,
        -56, -93, 48, 19, -8, 7, 6, -2,
        -17, 9, 7, -23, -3, -10, 5, 3,
        4, 9, -4, -5, 2, 2, -7, 3,
        -9, 7, 8, -6, 5, 12, 2, -5,
        -9, -4, -2, -3, 6, 1, -1, -1
    });

    auto true_result = from_vector({
        36, -13, 6, 1, 1, 0, 0, 0,
        -20, 11, -3, 2, 0, 0, 0, 0,
        8, -1, -3, 3, -1, 0, 0, 0,
        -4, -5, 2, 1, 0, 0, 0, 0,
        -1, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0
    });

    mat_byte result = quantize(input, y_table);
    CHECK_EQUAL_MAT(result, true_result);
}