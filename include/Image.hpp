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

static const auto debug = false;

struct Channel
{
    uint width, height;
    std::vector<Byte> pixels;

    // ctor
    Channel(uint w, uint h)
        : width{w},
        height{h}
    {
        pixels.reserve(width*height);
        if (debug) std::cout << "Channel::Channel()" << std::endl;
    }

    // copy ctor
    Channel(const Channel& other)
        : pixels(other.pixels),
        width(other.width), height(other.height)
    {
        if (debug) std::cout << "Channel::Channel(Channel&)" << std::endl;
    }

    // move ctor
    Channel(Channel&& other)
        : pixels(std::move(other.pixels)),
        width(other.width), height(other.height)
    {
        if (debug) std::cout << "Channel::Channel(Channel&&)" << std::endl;
    }

    // assignment
    Channel& operator=(const Channel &other) {
        if (this != &other) {
            pixels = other.pixels;
            width  = other.width;
            height = other.height;
            if (debug) std::cout << "Channel::operator=(Channel&)" << std::endl;
        }
        return *this;
    }

    // move assignment
    Channel& operator=(Channel &&other) {
        if (this != &other) {
            pixels = std::move(other.pixels);
            width  = other.width;
            height = other.height;
            if (debug) std::cout << "Channel::operator=(Channel&)" << std::endl;
        }
        return *this;
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
        if (debug) std::cout << "Image::Image()\n" << std::endl;
    }

    // copy ctor
    Image(const Image& other)
        : color_space_type(other.color_space_type),
        width(other.width), height(other.height),
        one(other.one), two(other.two), three(other.three),
        Y(one), Cb(two), Cr(three),
        R(one), G(two), B(three)
    {
        if (debug) std::cout << "Image::Image(Image&)\n" << std::endl;
    }

    // move ctor
    Image(Image&& other)
        : color_space_type(other.color_space_type),
        width(other.width), height(other.height),
        one(std::move(other.one)), two(std::move(other.two)), three(std::move(other.three)),
        Y(one), Cb(two), Cr(three),
        R(one), G(two), B(three)
    {
        if (debug) std::cout << "Image::Image(Image&&)\n" << std::endl;
    }

    // assignment
    Image& operator=(const Image &other) {
        if (this != &other) {
            one = other.one;
            two = other.two;
            three = other.three;
            width = other.width;
            height = other.height;
            color_space_type = other.color_space_type;
            if (debug) std::cout << "Image::operator=(Image&)\n" << std::endl;
        }
        return *this;
    }

    // move assignment
    Image& operator=(Image &&other) {
        if (this != &other) {
            one = std::move(other.one);
            two = std::move(other.two);
            three = std::move(other.three);
            width = other.width;
            height = other.height;
            color_space_type = other.color_space_type;
            if (debug) std::cout << "Image::operator=(Image&&)\n" << std::endl;
        }
        return *this;
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
