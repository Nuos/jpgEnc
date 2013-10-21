#pragma once

#include "Image.hpp"

//
// JPEG stuff
//
namespace Segment
{
    // Byte array with constant size T
    template <int T>
    using Bytes = std::array<Byte, T>;

    // setting the bytes in an byte array with a initializer list
    template <int sz>
    void set(Bytes<sz>& arr, std::initializer_list<Byte> list)
    {
        assert((list.size() <= sz) && "Trying to do a buffer overrun, eh?");
        std::copy(list.begin(), list.end(), arr.data());
    }

    // getting the high byte of a int
    inline Byte getHi(int i) { return (i & 0xFF00) >> 8; }
    inline Byte getLo(int i) { return (i & 0x00FF); }

    // stuff for the component (Y, Cb or Cr) config
    namespace CompSetup
    {
        enum ID : Byte
        {
            Y = 1,
            Cb,
            Cr
        };

        enum Subsampling : Byte
        {
            NoSubSampling = 0x22,
            Half = 0x11
        };
    }

    //static const Byte  mSOI[2] = { 0xff, 0xd8 };
    //static const Byte  mEOI[2] = { 0xff, 0xd9 };
    //static const Byte  mDHT[2] = { 0xff, 0xc4 };

    struct sAPP0
    {
        const Bytes<2> marker;
        Bytes<2> len;               // Fill that! HI/LO | Constraint: >= 16
        const Bytes<5> type;
        const Bytes<2> rev;
        const Bytes<1> pixelsize;
        Bytes<2> x_density;         // Fill that!
        Bytes<2> y_density;         // Fill that!
        const Bytes<2> thumbnail_size;

        // defaults
        sAPP0()
            : marker{ { 0xff, 0xe0 } },
            len{ { 0, 0 } },
            type{ { 'J', 'F', 'I', 'F', '\0' } },
            rev{ { 1, 1 } },
            pixelsize{ { 0 } },
            x_density{ { 0, 0 } },
            y_density{ { 0, 0 } },
            thumbnail_size{ { 0, 0 } }
        {}

        void setLen(int _len) { set(len, { getHi(_len), getLo(_len) }); }
        void setXdensity(int _den) { set(x_density, { getHi(_den), getLo(_den) }); }
        void setYdensity(int _den) { set(y_density, { getHi(_den), getLo(_den) }); }
    };
    static sAPP0 APP0; // prefilled/predefined APP0 segment

    template<uint num_components>
    struct sSOF0
    {
        const Bytes<2> marker;
        const Bytes<2> len;   // HI/LO
                              // Constraint: 8 + num components * 3
                              // 1 component (Y) or 3 components (YCbCr)
        const Bytes<1> precision;
        Bytes<2> image_size_x;       // Fill that! HI/LO >0!
        Bytes<2> image_size_y;       // Fill that! HI/LO >0!
        const Bytes<1> component_count;
        Bytes<num_components * 3> component_setup; // Fill that!

        // const defaults
        sSOF0()
            : marker{ { 0xff, 0xc0 } },
            len{ { 0, 8 + num_components * 3 } },
            precision{ { 8 } },
            image_size_x{ { 0, 0 } },
            image_size_y{ { 0, 0 } },
            component_count{ { num_components } },
            component_setup{ { 0 } }
        {}

        void setImageSizeX(int _sz) { set(image_size_x, { getHi(_sz), getLo(_sz) }); }
        void setImageSizeY(int _sz) { set(image_size_y, { getHi(_sz), getLo(_sz) }); }
        void setCompSetup(std::initializer_list<Byte> comp_setup) { set(component_setup, comp_setup); }
    };
    static sSOF0<1> SOF0_1c; // prefilled/predefined SOF0 segment with 1 component/channel (Y, grayscale)
    static sSOF0<3> SOF0_3c; // prefilled/predefined SOF0 segment with 3 components/channels (color image)
}
