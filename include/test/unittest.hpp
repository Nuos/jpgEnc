#pragma once

#include <cassert>
#include <boost/test/unit_test.hpp>

#include <boost/numeric/ublas/matrix.hpp>
using boost::numeric::ublas::matrix;

#define _t_str std::to_string

// check if two floating point numbers are equal to the 5th digit
const auto delta = .00001;
#define CHECK_CLOSE(left, right) if (abs((left) - (right)) >= delta) BOOST_ERROR(_t_str(left) + " not close to " + _t_str(right) + " (delta: " + _t_str(delta) + ")")

#define CHECK_EQUAL_MAT(left, right) { \
    bool size_equal = left.size1() == right.size1() && left.size2() == right.size2(); \
    \
    if (!size_equal) BOOST_ERROR("left size (" + _t_str(left.size1()) + "/" + _t_str(left.size2()) + ") doesn't match right size (" + _t_str(right.size1()) + "/" + _t_str(right.size2()) + ")"); \
    \
    for (size_t i = 0; i < left.size1(); ++i) { \
        for (size_t j = 0; j < left.size2(); ++j) { \
            if (abs(left(i, j) - right(i, j)) >= delta) BOOST_ERROR("Value at position (" + _t_str(i) + "," + _t_str(j) + "): " + _t_str(left(i, j)) + " not close to " + _t_str(right(i, j)) + " (delta: " + _t_str(delta) + ")"); \
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
