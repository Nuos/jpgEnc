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
#include <array>

typedef unsigned int uint;
typedef uint8_t Byte;

class Image;

// load a ppm file (P3 or P6 version)
Image loadPPM(std::string path);


// image class handling three channels (RGB, YUV, whatever) with one byte pixels
class Image
{
    // NESTED CHANNEL TYPE
private:
    struct Channel
    {
        uint width, height;
        std::vector<Byte> pixels;

        Channel(uint w, uint h); // ctor
        Channel(const Channel& other); // copy ctor
        Channel(Channel&& other); // move ctor

        Channel& operator=(const Channel &other); // assignment
        Channel& operator=(Channel &&other); // move assignment

        // for one-dimensional indexing
        Byte& operator()(uint x);

        // for two-dimensional indexing
        // stops at pixel border, so no out-of-bounds indexing possible
        // practically duplicates pixel at the border 
        Byte& operator()(uint x, uint y);
    };

    // NESTED ENUMS
public:
    enum ColorSpace
    {
        RGB,
        YCbCr
    };

    enum SubsamplingMode
    {
        S444,       // full sampling
        S422,       // every second pixel in a row
        S411,       // every fourth pixel in a row
        S420,       // every second pixel in every second row
        S420_m,     // between vertical and horizontal pixels
        S420_lm,    // between vertical pixels
    };

    // INTERFACE
public:
    // CTORS
    explicit Image(uint w, uint h, ColorSpace color);   // ctor
    Image(const Image& other);                          // copy ctor
    Image(Image&& other);                               // move ctor

    ~Image(); // dtor

    // ASSIGNMENTS
    Image& operator=(const Image &other);   // copy assignment
    Image& operator=(Image &&other);        // move assignment

    // METHODS
    // returns a new image object, this object won't be modified
    Image convertToColorSpace(ColorSpace target_space) const;

    // apply subsampling to the color channels (Cb, Cr)
    void applySubsampling(SubsamplingMode mode);

    // JPEG SEGMENTS
    void writeJPEG(std::wstring file);

    // HELPER
private:
    struct Mask;
    void subsample(Channel&, int, int, Mask&, bool, SubsamplingMode);

    // ACCESSORS
public:
    uint width, height;
    Channel &R, &G, &B;
    Channel &Y, &Cb, &Cr;

    // HIDDEN MEMBERS
private:
    ColorSpace color_space_type;
    Channel one, two, three;
};