#pragma once

#include "Image.hpp"
#include <ostream>

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
    inline Byte getHi(short i) { return (i & 0xFF00) >> 8; }
    inline Byte getLo(short i) { return (i & 0x00FF); }

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

    struct sSOI
    {
        const Bytes<2> marker;

        // defaults
        sSOI()
            : marker{ { 0xff, 0xd8 } }
        {}

        // stream I/O
        friend std::ostream& operator<<(std::ostream& out, const sSOI& SOI)
        {
            const auto& segment = SOI;
            out.write((const char*)&segment, sizeof(segment));
            return out;
        }
    };
    static sSOI SOI;

    struct sEOI
    {
        const Bytes<2> marker;

        // defaults
        sEOI()
            : marker{ { 0xff, 0xd9 } }
        {}

        // stream I/O
        friend std::ostream& operator<<(std::ostream& out, const sEOI& EOI)
        {
            const auto& segment = EOI;
            out.write((const char*)&segment, sizeof(segment));
            return out;
        }
    };
    static sEOI EOI;

    struct sAPP0
    {
        const Bytes<2> marker;
        Bytes<2> len;               // Fill that! HI/LO | Constraint: >= 16, without marker
        const Bytes<5> type;
        const Bytes<2> rev;
        const Bytes<1> pixelsize;
        Bytes<2> x_density;         // Fill that!
        Bytes<2> y_density;         // Fill that!
        const Bytes<2> thumbnail_size;

        // defaults
        sAPP0()
            : marker{ { 0xff, 0xe0 } },
            len{ { 0, 16 } }, // length without thumbnail
            type{ { 'J', 'F', 'I', 'F', '\0' } },
            rev{ { 1, 1 } },
            pixelsize{ { 0 } },
            x_density{ { 0, 0x48 } }, // arbitrary value?!
            y_density{ { 0, 0x48 } }, // arbitrary value?!
            thumbnail_size{ { 0, 0 } }
        {}

        // setter
        sAPP0& setLen(short _len) { set(len, { getHi(_len), getLo(_len) }); return *this; }
        sAPP0& setXdensity(short _den) { set(x_density, { getHi(_den), getLo(_den) }); return *this; }
        sAPP0& setYdensity(short _den) { set(y_density, { getHi(_den), getLo(_den) }); return *this; }

        // stream I/O
        friend std::ostream& operator<<(std::ostream& out, const sAPP0& APP0)
        {
            const auto& segment = APP0;
            out.write((const char*)&segment, sizeof(segment));
            return out;
        }
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
        Bytes<2> image_size_y;       // Fill that! HI/LO >0!
        Bytes<2> image_size_x;       // Fill that! HI/LO >0!
        const Bytes<1> component_count;
        Bytes<num_components * 3> component_setup; // Fill that!

        // defaults
        sSOF0()
            : sSOF0{ 0, 0, {} }
        {}

        // parametrized constructor
        sSOF0(int size_x,
              int size_y,
              std::initializer_list<Byte> comp_setup = {
                  CompSetup::Y, CompSetup::NoSubSampling, 0,
                  CompSetup::Cb, CompSetup::Half, 1,
                  CompSetup::Cr, CompSetup::Half, 2,
              }
        )
            : marker{ { 0xff, 0xc0 } },
            len{ { 0, 8 + num_components * 3 } },
            precision{ { 8 } },
            component_count{ { num_components } },
            image_size_x{ { getHi(size_x), getLo(size_x) } },
            image_size_y{ { getHi(size_y), getLo(size_y) } }
        {
            set(component_setup, comp_setup);
        }

        // setter
        sSOF0& setImageSizeX(short _sz) { set(image_size_x, { getHi(_sz), getLo(_sz) }); return *this; }
        sSOF0& setImageSizeY(short _sz) { set(image_size_y, { getHi(_sz), getLo(_sz) }); return *this; }
        sSOF0& setCompSetup(std::initializer_list<Byte> comp_setup) { set(component_setup, comp_setup); return *this; }

        // stream I/O
        friend std::ostream& operator<<(std::ostream& out, const sSOF0<num_components>& SOF0)
        {
            const auto& segment = SOF0;
            out.write((const char*)&segment, sizeof(segment));
            return out;
        }
    };
    static sSOF0<1> SOF0_1c; // prefilled/predefined SOF0 segment with 1 component/channel (Y, grayscale)
    static sSOF0<3> SOF0_3c; // prefilled/predefined SOF0 segment with 3 components/channels (color image)

    struct sDHT
    {
        static const auto fixed_len = 19;
        const Bytes<2> marker;
        Bytes<2> len;               // Fill that! Length is without this 2-Byte length
        const Bytes<1> HT_info;
        Bytes<16> code_lengths;     // Fill that!
        std::vector<Byte> symbols;

        // defaults
        sDHT()
            : marker{ { 0xff, 0xc4 } },
            len{ { 0, 0 } },
            HT_info{ { 0 } },
            code_lengths{ { 0 } },
            symbols{ {} }
        {}

        // setter
        sDHT& setLen(short _len) { set(len, { getHi(_len), getLo(_len) }); return *this; }
        sDHT& setCodeData(/* _symbol_and_its_length_or_something_like_that */) {
            // symbole sortiert nach
            //   1. codelänge aufsteigend
            //   2. alphabet aufsteigend
            symbols = std::vector<Byte>{{ 0, 4, 9, 1, 7 }};
            code_lengths[3] = 2;
            code_lengths[2] = 3;

            recalcLength();

            return *this;
        }
        sDHT& recalcLength() {
            assert(symbols.size() <= 256);
            setLen(static_cast<short>(fixed_len + symbols.size()));
            return *this;
        }

        // stream I/O
        friend std::ostream& operator<<(std::ostream& out, const sDHT& DHT)
        {
            const auto& segment = DHT;
            // write fixed size portion of the segment
            out.write((const char*)&segment, fixed_len + 2);
            // then write the varying symbols
            out.write((const char*)&segment.symbols[0], segment.symbols.size());
            return out;
        }
    };
    static sDHT DHT; // prefilled/predefined DHT segment
}
