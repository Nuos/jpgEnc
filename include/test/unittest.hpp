#pragma once

#include <cassert>
#include <boost/test/unit_test.hpp>

#include <boost/numeric/ublas/matrix.hpp>
using boost::numeric::ublas::matrix;

#define _str std::to_string

// check if two floating point numbers are equal to the 5th digit
const auto delta = .00001;
#define CHECK_CLOSE(left, right) if (abs(left - right) >= delta) BOOST_ERROR(_str(left) + " not close to " + _str(right) + " (delta: " + _str(delta) + ")")

//template<typename T>
//void CHECK_EQUAL(const matrix<T>& m, const matrix<T>& n) {
//    bool size_equal = m.size1() == n.size1() && m.size2() == n.size2();
//    BOOST_CHECK(size_equal);
//
//    if (!size_equal)
//        return;
//
//    for (size_t i = 0; i < m.size1(); ++i) {
//        for (size_t j = 0; j < m.size2(); ++j) {
//            CHECK_CLOSE(m(i, j), n(i, j));
//        }
//    }
//}

#define CHECK_EQUAL_MAT(left, right) { \
    bool size_equal = left.size1() == right.size1() && left.size2() == right.size2(); \
    \
    if (!size_equal) BOOST_ERROR("left size (" + _str(left.size1()) + "/" + _str(left.size2()) + ") doesn't match right size (" + _str(right.size1()) + "/" + _str(right.size2()) + ")"); \
    \
    for (size_t i = 0; i < left.size1(); ++i) { \
        for (size_t j = 0; j < left.size2(); ++j) { \
            if (abs(left(i, j) - right(i, j)) >= delta) BOOST_ERROR("Value at position (" + _str(i) + "," + _str(j) + "): " + _str(left(i, j)) + " not close to " + _str(right(i, j)) + " (delta: " + _str(delta) + ")"); \
        } \
    } } 

// print boost::matrix beatifully
template <typename T>
void printMat(const matrix<T> &mat) {
    for (auto i = 0u; i < mat.size1(); ++i) {
        for (auto j = 0u; j < mat.size2(); ++j) {
            printf("%6.3f,", mat(i, j));
        }
        printf("\n");
    }
    printf("\n");
}

#undef str

// Example testcase
//int add(int i, int j) { return i+j; }
//BOOST_AUTO_TEST_CASE(test_add) {
//    // seven ways to detect and report the same error:
//    BOOST_CHECK(add(2, 2) == 4);        // #1 continues on error
//
//    BOOST_REQUIRE(add(2, 2) == 4);      // #2 throws on error
//
//    if (add(2, 2) != 4)
//        BOOST_ERROR("Ouch...");            // #3 continues on error
//
//    if (add(2, 2) != 4)
//        BOOST_FAIL("Ouch...");             // #4 throws on error
//
//    if (add(2, 2) != 4) throw "Ouch..."; // #5 throws on error
//
//    BOOST_CHECK_MESSAGE(add(2, 2) == 4,  // #6 continues on error
//                        "add(..) result: " << add(2, 2));
//
//    BOOST_CHECK_EQUAL(add(2, 2), 4);	  // #7 continues on error
//}
