#include "test/unittest.hpp"

#include <boost/numeric/ublas/io.hpp>
#include "Dct.hpp"

using mat = matrix<PixelDataType>;

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