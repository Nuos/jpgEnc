#include <boost/test/unit_test.hpp>

#include <boost/numeric/ublas/io.hpp>
#include "Dct.hpp"

template<typename T>
void CHECK_EQUAL(const matrix<T>& m, const matrix<T>& n) {
    bool size_equal = m.size1() == n.size1() && m.size2() == n.size2();
    BOOST_CHECK(size_equal);

    if (!size_equal)
        return;

    for (size_t i = 0; i < m.size1(); ++i) {
        for (size_t j = 0; j < m.size2(); ++j) {
            BOOST_CHECK_CLOSE(m(i, j), n(i, j), 0.0001);
        }
    }
}

matrix<PixelDataType> mat(const std::vector<PixelDataType>& v) {
    assert(v.size() == 64);

    matrix<PixelDataType> m(8, 8);
    for (size_t i = 0; i < m.size1(); i++) {
        for (size_t j = 0; j < m.size2(); j++) {
            m(i, j) = v[i*m.size2() + j];
        }
    }
    return m;
}

BOOST_AUTO_TEST_CASE(arai) {
    auto m = mat({
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

    auto true_dct = mat({
        260, -18.2216411837961, 7.69085915161152e-15, -1.90481782616726, 0, -0.568239222367164, 1.85673764701218e-14, -0.143407824981022,
        -145.773129470369, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        -15.2385426093380, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        -4.54591377893732, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        -1.14726259984816, 0, 0, 0, 0, 0, 0, 0
    });

    auto dct = dctArai(m);
    auto dct2 = dctArai2(m);

    std::cout << dct2 << std::endl;

    CHECK_EQUAL(dct, true_dct);
    CHECK_EQUAL(dct2, true_dct);
}