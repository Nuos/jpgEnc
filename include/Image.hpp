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

typedef uint8_t byte;
typedef unsigned int uint;

enum ColorSpace {
    RGB,
    YCbCr
};


struct Channel {
    uint width;
    uint height;
    std::vector<byte> pixels;

    Channel(uint width, uint height) 
        : width(width), 
          height(width),
          pixels(width * height)
    {
        assert(width > 0);
        assert(height > 0);
    }

    Channel(Channel&& other)
        : width(other.width),
          height(other.height),
          pixels(std::move(other.pixels))
    {
    }

    Channel& operator=(Channel&& other) {
        width = other.width;
        height = other.height;
        pixels = std::move(other.pixels);
    }

    // x along width, y along height
    byte& at(uint x, uint y) {
        x = std::min(width - 1, x);
        y = std::min(height - 1, y);
        return pixels[y * width + x];
    }
};


class Image {
public:
    Image(uint width, uint height, ColorSpace color_space)
        : ch1(width, height),
          ch2(width, height),
          ch3(width, height),
          r(ch1), g(ch2), b(ch3),
          y(ch1), cb(ch2), cr(ch3),
          width(width),
          height(height),
          color_space(color_space)
    {
    }

    Image(const Image& other)
        : ch1(other.ch1),
          ch2(other.ch2),
          ch3(other.ch3),
          r(ch1), g(ch2), b(ch3),
          y(ch1), cb(ch2), cr(ch3),
          width(other.width),
          height(other.height),
          color_space(other.color_space)
    {
    }


    Image(const Image&& other)
        : ch1(std::move(other.ch1)),
          ch2(std::move(other.ch2)),
          ch3(std::move(other.ch3)),
          r(ch1), g(ch2), b(ch3),
          y(ch1), cb(ch2), cr(ch3),
          width(other.width),
          height(other.height),
          color_space(other.color_space)
    {
    }

    Image& operator=(const Image& other) {
        if (this != &other) {
            // r, g, b etc references already point to the correct channels
            ch1 = other.ch1;
            ch2 = other.ch2;
            ch3 = other.ch3;
            width = other.width;
            height = other.height;
            color_space = other.color_space;
        }
        return *this;
    }

    Image& operator=(const Image&& other) {
        if (this != &other) {
            // r, g, b etc references already point to the correct channels
            ch1 = std::move(other.ch1);
            ch2 = std::move(other.ch2);
            ch3 = std::move(other.ch3);
            width = other.width;
            height = other.height;
            color_space = other.color_space;
        }
        return *this;
    }

    Image convertToColorSpace(ColorSpace color_space) const;

public:
    Channel& r;
    Channel& g;
    Channel& b;

    Channel& y;
    Channel& cb;
    Channel& cr;

    uint width;
    uint height;
    ColorSpace color_space;

private:
    Channel ch1, ch2, ch3;
};


// load a ppm file (P3 or P6 version)
Image loadPPM(std::string path);
