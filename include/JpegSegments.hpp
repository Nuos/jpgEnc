#pragma once

#include "Image.hpp"
#include <ostream>
#include <vector>

//
// JPEG stuff
//
namespace Segment
{
    using std::vector;

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
        const Bytes<2> marker;
        Bytes<2> len;               // Fill that! Length is without this 2-Byte length
        struct sHT {
            Bytes<1> HT_info; 
            Bytes<16> code_lengths;
            std::vector<Byte> symbols;
        };
        std::vector<sHT> HTs;

        enum Class { DC = 0, AC };
        enum Destination { First = 0, Second };

        // defaults
        sDHT()
            : marker{ { 0xff, 0xc4 } },
            len{ { 0, 0 } }
        {}

        // setter
        sDHT& pushCodeData(vector<vector<int>> &codelength_symbols, Class cls, Destination dest) {
            // codelength_symbols[0] is the symbol list with codelength 0
            // codelength_symbols[1] is the symbol list with codelength 1
            // ...
            // codelength_symbols[16] is the symbol list with codelength 16 (MAX!)
            // 
            assert(codelength_symbols.size() == 17);

            HTs.resize(HTs.size() + 1);

            auto& HTinfo = HTs.back().HT_info;
            auto& symbols = HTs.back().symbols;
            auto& code_lengths = HTs.back().code_lengths;

            HTinfo.assign(((Byte)cls << 4) | (Byte)dest);

            // symbols with codelength 0 shouldn't be possible and the DHT segment also starts with codelength 1
            symbols.clear();
            for (int i = 1; i < codelength_symbols.size(); ++i) {
                auto& symbol_list = codelength_symbols[i];

                // symbol order is arbitrary, see itu-t81.pdf Page 51

                assert(symbol_list.size() < 256);
                code_lengths[i-1] = static_cast<Byte>(symbol_list.size());
                symbols.insert(end(symbols), begin(symbol_list), end(symbol_list));
            }

            recalcLength();
            return *this;
        }

        sDHT& clear() {
            HTs.clear();
            recalcLength();
            return *this;
        }

        // stream I/O
        friend std::ostream& operator<<(std::ostream& out, const sDHT& DHT)
        {
            const auto& segment = DHT;
            // write marker and length
            out.write((const char*)&segment, 4);

            // then write the HTs
            for (const auto& HT : segment.HTs) {
                // HT info and code length array
                out.write((const char*)&HT, 17);

                // symbols
                out.write((const char*)&HT.symbols[0], HT.symbols.size());
            }
            return out;
        }

    private:
        sDHT& setLen(short _len) { set(len, { getHi(_len), getLo(_len) }); return *this; }
        sDHT& recalcLength() {
            auto len = 2;
            for (const auto& HT : HTs) {
                assert(HT.symbols.size() <= 256);
                len += 17 + HT.symbols.size();
            }
            assert(len < 65536); // only 16 bit available in length field
            setLen(static_cast<short>(len));
            return *this;
        }
    };
    static sDHT DHT; // prefilled/predefined DHT segment

    struct sDQT
    {
        const Bytes<2> marker;
        Bytes<2> len;               // Fill that! Length is without this 2-Byte length
        struct sQT {
            Bytes<1> QT_info;
            std::array<Byte, 64> coefficients;
        };
        std::vector<sQT> QTs;

        enum Destination { Zero = 0, One, Two, Three };

        // defaults
        sDQT()
            : marker{ { 0xff, 0xdb } },
            len{ { 0, 0 } }
        {}

        // setter
        sDQT& pushQuantizationTable(vector<Byte> &coefficients, Destination dest) {
            // add new table
            QTs.resize(QTs.size() + 1);
            auto& QT = QTs.back();

            QT.QT_info.assign((Byte)dest);

            assert(coefficients.size() == 64);
            for (auto i = 0u; i < coefficients.size(); ++i)
                QT.coefficients.at(i) = coefficients[i];

            recalcLength();
            return *this;
        }

        sDQT& clear() {
            QTs.clear();
            recalcLength();
            return *this;
        }

        // stream I/O
        friend std::ostream& operator<<(std::ostream& out, const sDQT& DQT)
        {
            // marker and length
            out.write((const char*)&DQT, 4);
            // QTs
            for (const auto& QT : DQT.QTs) {
                out.write((const char*)&QT.QT_info, 1); // QT info
                out.write((const char*)&QT.coefficients[0], QT.coefficients.size()); // coefficients
            }

            return out;
        }

    private:
        sDQT& setLen(short _len) { set(len, { getHi(_len), getLo(_len) }); return *this; }
        sDQT& recalcLength() {
            auto len = 2 + QTs.size() * 65;
            assert(len < 65536); // only 16 bit available in length field
            setLen(static_cast<short>(len));
            return *this;
        }
    };
    static sDQT DQT; // prefilled/predefined DHT segment
}
