#pragma once

#include <cassert>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/math/constants/constants.hpp>

#include "Image.hpp"

using boost::numeric::ublas::matrix;
using boost::numeric::ublas::trans;
using boost::math::constants::pi;
using boost::math::constants::root_two;
using std::cos;


const PixelDataType _pi = pi<PixelDataType>();
const PixelDataType c1 = cos(1 * _pi / 16);
const PixelDataType c2 = cos(2 * _pi / 16);
const PixelDataType c3 = cos(3 * _pi / 16);
const PixelDataType c4 = cos(4 * _pi / 16);
const PixelDataType c5 = cos(5 * _pi / 16);
const PixelDataType c6 = cos(6 * _pi / 16);
const PixelDataType c7 = cos(7 * _pi / 16);

const PixelDataType a1 = c4;
const PixelDataType a2 = c2 - c6;
const PixelDataType a3 = c4;
const PixelDataType a4 = c6 + c2;
const PixelDataType a5 = c6;

const PixelDataType s0 = 1 / (2 * root_two<PixelDataType>());
const PixelDataType s1 = 1 / (4 * c1);
const PixelDataType s2 = 1 / (4 * c2);
const PixelDataType s3 = 1 / (4 * c3);
const PixelDataType s4 = 1 / (4 * c4);
const PixelDataType s5 = 1 / (4 * c5);
const PixelDataType s6 = 1 / (4 * c6);
const PixelDataType s7 = 1 / (4 * c7);


void dctAraiSchema(matrix<PixelDataType>& x);

matrix<PixelDataType> dctArai(const matrix<PixelDataType>& x) {
    matrix<PixelDataType> y = x;
    dctAraiSchema(y);
    y = trans(y);
    dctAraiSchema(y);
    return trans(y);
}

void dctAraiSchema(matrix<PixelDataType>& x) {
    assert(x.size1() == 8 && x.size2() == 8);

    // A*x
    // columnwise through xdct
    for (uint j = 0; j < 8; j++) {
        auto x0 = x(0, j);
        auto x1 = x(1, j);
        auto x2 = x(2, j);
        auto x3 = x(3, j);
        auto x4 = x(4, j);
        auto x5 = x(5, j);
        auto x6 = x(6, j);
        auto x7 = x(7, j);

        auto z0 = x0 + x7;
        auto z1 = x1 + x6;
        auto z2 = x2 + x5;
        auto z3 = x3 + x4;
        auto z4 = -x4 + x3;
        auto z5 = -x5 + x2;
        auto z6 = -x6 + x1;
        auto z7 = -x7 + x0;

        auto r0 = z0 + z3;
        auto r1 = z1 + z2;
        auto r2 = z1 - z2;
        auto r3 = z0 - z3;
        auto r4 = -z4 - z5;
        auto r5 = z5 + z6;
        auto r6 = z6 + z7;
        auto r7 = z7;

        auto t0 = r0 + r1;
        auto t1 = r0 - r1;
        auto t2 = r2 + r3;
        auto t3 = r3;
        auto t4 = r4;
        auto t5 = r5;
        auto t6 = r6;
        auto t7 = r7;

        auto tmp = (t4 + t6) * a5;

        t2 *= a1;
        t4 *= a2;
        t5 *= a3;
        t6 *= a4;

        auto u0 = t0;
        auto u1 = t1;
        auto u2 = t2;
        auto u3 = t3;
        auto u4 = -t4 - tmp;
        auto u5 = t5;
        auto u6 = t6 - tmp;
        auto u7 = t7;

        auto v0 = u0;
        auto v1 = u1;
        auto v2 = u2 + u3;
        auto v3 = u3 - u2;
        auto v4 = u4;
        auto v5 = u5 + u7;
        auto v6 = u6;
        auto v7 = u7 - u5;

        auto w0 = v0;
        auto w1 = v1;
        auto w2 = v2;
        auto w3 = v3;
        auto w4 = v4 + v7;
        auto w5 = v5 + v6;
        auto w6 = -v6 + v5;
        auto w7 = v7 - v4;

        x(0, j) = w0 * s0;
        x(4, j) = w1 * s4;
        x(2, j) = w2 * s2;
        x(6, j) = w3 * s6;
        x(5, j) = w4 * s5;
        x(1, j) = w5 * s1;
        x(7, j) = w6 * s7;
        x(3, j) = w7 * s3;
    }
}


matrix<PixelDataType> dctArai2(matrix<PixelDataType> x) {
    assert(x.size1() == 8 && x.size2() == 8);

    matrix<PixelDataType> y(8, 8);

    for (uint k = 0; k < 2; k++) {
        for (uint j = 0; j < 8; j++) {
            auto x0 = x(0, j);
            auto x1 = x(1, j);
            auto x2 = x(2, j);
            auto x3 = x(3, j);
            auto x4 = x(4, j);
            auto x5 = x(5, j);
            auto x6 = x(6, j);
            auto x7 = x(7, j);

            auto z0 = x0 + x7;
            auto z1 = x1 + x6;
            auto z2 = x2 + x5;
            auto z3 = x3 + x4;
            auto z4 = -x4 + x3;
            auto z5 = -x5 + x2;
            auto z6 = -x6 + x1;
            auto z7 = -x7 + x0;

            auto r0 = z0 + z3;
            auto r1 = z1 + z2;
            auto r2 = z1 - z2;
            auto r3 = z0 - z3;
            auto r4 = -z4 - z5;
            auto r5 = z5 + z6;
            auto r6 = z6 + z7;
            auto r7 = z7;

            auto t0 = r0 + r1;
            auto t1 = r0 - r1;
            auto t2 = r2 + r3;
            auto t3 = r3;
            auto t4 = r4;
            auto t5 = r5;
            auto t6 = r6;
            auto t7 = r7;

            auto tmp = (t4 + t6) * a5;

            t2 *= a1;
            t4 *= a2;
            t5 *= a3;
            t6 *= a4;

            auto u0 = t0;
            auto u1 = t1;
            auto u2 = t2;
            auto u3 = t3;
            auto u4 = -t4 - tmp;
            auto u5 = t5;
            auto u6 = t6 - tmp;
            auto u7 = t7;

            auto v0 = u0;
            auto v1 = u1;
            auto v2 = u2 + u3;
            auto v3 = u3 - u2;
            auto v4 = u4;
            auto v5 = u5 + u7;
            auto v6 = u6;
            auto v7 = u7 - u5;

            auto w0 = v0;
            auto w1 = v1;
            auto w2 = v2;
            auto w3 = v3;
            auto w4 = v4 + v7;
            auto w5 = v5 + v6;
            auto w6 = -v6 + v5;
            auto w7 = v7 - v4;

            // also transposes
            y(j, 0) = w0 * s0;
            y(j, 4) = w1 * s4;
            y(j, 2) = w2 * s2;
            y(j, 6) = w3 * s6;
            y(j, 5) = w4 * s5;
            y(j, 1) = w5 * s1;
            y(j, 7) = w6 * s7;
            y(j, 3) = w7 * s3;
        }
        if (k==0)
            x = y;
    }
    return y;
}