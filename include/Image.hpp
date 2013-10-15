#pragma once

#include <cassert>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <string>
#include <exception>
#include <fstream>
#include <cctype>
#include <iostream>

typedef unsigned int uint;
typedef uint8_t Byte;

struct Channel
{
    const uint width, height;
    std::vector<Byte> pixels;

    Channel(uint w, uint h)
        : width{w},
        height{h}
    {
        pixels.reserve(width*height);
    }

    // for one-dimensional indexing
    Byte& at(uint x) {
        return pixels[x];
    }

    // for two-dimensional indexing
    Byte& at(uint x, uint y) {
        x = std::min(x, width-1);
        y = std::min(y, height-1);

        return pixels[y * width + x];
    }
};

enum ColorSpace
{
    RGB,
    YCbCr
};

// image class handling three channels (RGB, YUV, whatever) with one byte pixels
class Image
{
public:
    // ctor
    explicit Image(uint w, uint h, ColorSpace color)
        : color_space_type(color),
        width(w), height(h),
        one(w, h), two(w, h), three(w, h),
        Y(one), Cb(two), Cr(three),
        R(one), G(two), B(three)
    {
        std::cout << "Image::Image()" << std::endl;
    }

    // copy ctor
    Image(const Image& other)
        : color_space_type(other.color_space_type),
        width(other.width), height(other.height),
        one(other.one), two(other.two), three(other.three),
        Y(one), Cb(two), Cr(three),
        R(one), G(two), B(three)
    {
        std::cout << "Image::Image(Image&)" << std::endl;
    }

    Image(Image&& other)
        : color_space_type(other.color_space_type),
        width(other.width), height(other.height),
        one(std::move(other.one)), two(std::move(other.two)), three(std::move(other.three)),
        Y(one), Cb(two), Cr(three),
        R(one), G(two), B(three)
    {
        std::cout << "Image::Image(Image&&)" << std::endl;
    }

    // returns a new image object, this object won't be modified
    Image convertToColorSpace(ColorSpace target_space) const;

    uint width, height;
    Channel &R, &G, &B;
    Channel &Y, &Cb, &Cr;

private:
    ColorSpace color_space_type;
    Channel one, two, three;
};

// load a ppm file (P3 or P6 version)
Image loadPPM(std::string path);