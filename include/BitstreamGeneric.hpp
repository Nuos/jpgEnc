#pragma once

#include <cstdint>
#include <vector>
#include <initializer_list>
#include <ostream>
#include <istream>

// Bitstream_Generic[0] is the least significant bit (LSB)
// Bitsream[Bitstream_Generic.size()-1] is the most significant bit (MSB)
template <typename BlockType>
class Bitstream_Generic
{
public:
    friend class BitView;
    static const auto block_size = sizeof(BlockType) * 8;

    typedef std::vector<BlockType> ContainerType;

    class BitView
    {

        friend class Bitstream_Generic<BlockType>;
        BitView(ContainerType& bstream, unsigned int block_idx, uint8_t bit_idx)
            : bits(bstream),
            block_index(block_idx),
            bit_index(bit_idx)
        {}

        ContainerType& bits;
        const unsigned int block_index;
        const uint8_t bit_index;

    public:
        operator bool()
        {
            return (bits[block_index] & ((BlockType) 1 << bit_index)) != 0;
        }

        void operator=(bool val)
        {
            auto& block = bits[block_index];
            if (val) {
                // set bit at bit_idx
                auto mask = (BlockType) 1 << bit_index;
                block |= mask;
            }
            else {
                // unset bit at bit_idx
                auto mask = ~((BlockType) 1 << bit_index);
                block &= mask;
            }
        }
    };

private:
    ContainerType bits;
    uint8_t bit_idx;
    size_t sz;

private:
    // appending Blocks, don't let the user see this as it needs proper alignment,
    // only used for reading in a file 
    Bitstream_Generic& operator<<(BlockType val);

public:
    // ctors
    Bitstream_Generic();
    Bitstream_Generic(std::initializer_list<bool> args);

    // appending bits
    Bitstream_Generic& operator<<(bool val);
    Bitstream_Generic& operator<<(std::initializer_list<bool> args);

    // streaming
    template<typename BlockType>
    friend std::ostream& operator<<(std::ostream& out, const Bitstream_Generic<BlockType>& bitstream);
    template<typename BlockType>
    friend std::istream& operator>>(std::istream& in, Bitstream_Generic<BlockType>& bitstream);

    // accessing
    BitView operator[](unsigned int pos);

    // others
    size_t size() const;
};

//
// Implementation
//
template<typename BlockType>
Bitstream_Generic<BlockType>::Bitstream_Generic()
: bit_idx(block_size),
bits(),
sz(0)
{}

template<typename BlockType>
Bitstream_Generic<BlockType>::Bitstream_Generic(std::initializer_list<bool> args)
: Bitstream_Generic()
{
    *this << args;
}

template<typename BlockType>
Bitstream_Generic<BlockType>& Bitstream_Generic<BlockType>::operator<<(bool val)
{
    // make bit stream longer if we run out of space
    if (bit_idx >= block_size) {
        bits.push_back(0);
        bit_idx = block_size - 1;
    }

    auto& block = bits.back();
    if (val) {
        // set bit at bit_idx
        auto mask = (BlockType) 1 << bit_idx;
        block |= mask;
    }
    --bit_idx;
    ++sz;

    return *this;
}

template<typename BlockType>
Bitstream_Generic<BlockType>& Bitstream_Generic<BlockType>::operator<<(BlockType val)
{
    bits.push_back(val);
    sz += block_size;
    return *this;
}

template<typename BlockType>
Bitstream_Generic<BlockType>& Bitstream_Generic<BlockType>::operator<<(std::initializer_list<bool> args)
{
    // todo: perf improve: use operator<<(bool) as long as the block is not aligned
    //          then use operator<<(BlockType) if enough bits are available
    for (auto& bit : args)
        *this << bit;
    return *this;
}

template<typename BlockType>
std::ostream& operator<<(std::ostream& out, const Bitstream_Generic<BlockType>& bitstream)
{
    for (const auto& bit : bitstream.bits)
        out.write((const char*)&bit, sizeof(bit));
    return out;
}

template<typename BlockType>
std::istream& operator>>(std::istream& in, Bitstream_Generic<BlockType>& bitstream)
{
    BlockType b = 0;
    while (in.read((char*) &b, sizeof(b)))
        bitstream << b;
    return in;
}

template<typename BlockType>
size_t Bitstream_Generic<BlockType>::size() const
{
    return sz;
}

template<typename BlockType>
typename Bitstream_Generic<BlockType>::BitView Bitstream_Generic<BlockType>::operator[](unsigned int pos) {
    auto block_idx = static_cast<unsigned int>(pos / block_size);
    auto bit_idx = static_cast<uint8_t>(block_size - (pos - block_idx * block_size) - 1);

    return BitView(bits, block_idx, bit_idx);
}

// Convenience typedefs
typedef Bitstream_Generic<uint8_t> Bitstream8;
typedef Bitstream_Generic<uint16_t> Bitstream16;
typedef Bitstream_Generic<uint32_t> Bitstream32;
typedef Bitstream_Generic<uint64_t> Bitstream64;
typedef Bitstream64 Bitstream;

typedef std::initializer_list<bool> Bits;