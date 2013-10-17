#pragma once

#include <cstdint>
#include <vector>
#include <deque>
#include <initializer_list>
#include <algorithm>
#include <ostream>
#include <istream>

// BitstreamL[0] is the least significant bit (LSB)
// Bitsream[BitstreamL.size()-1] is the most significant bit (MSB)
class BitstreamL
{
public:
    friend class BitView;
    typedef uint64_t BlockType;
    static const auto block_size = sizeof(BlockType) * 8;

    class BitView
    {
        friend class BitstreamL;
        BitView(std::deque<BlockType>& bstream, size_t block_idx, uint8_t bit_idx)
            : bits(bstream),
            block_index(block_idx),
            bit_index(bit_idx)
        {}

        std::deque<BlockType>& bits;
        const size_t block_index;
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
    std::deque<BlockType> bits;
    uint8_t bit_idx;
    size_t sz;

private:
    // appending Blocks, don't let the user see this as it needs proper alignment,
    // only used for reading in a file 
    BitstreamL& operator<<(BlockType val);

public:
    // ctors
    BitstreamL();
    BitstreamL(std::initializer_list<bool> args);

    // appending bits
    BitstreamL& operator<<(bool val);
    BitstreamL& operator<<(std::initializer_list<bool> args);

    // streaming
    friend std::ostream& operator<<(std::ostream& out, const BitstreamL& bitstream);
    friend std::istream& operator>>(std::istream& in, BitstreamL& bitstream);

    // accessing
    BitView operator[](unsigned int pos);

    // others
    size_t size() const;
};

//
// Implementation
//
BitstreamL::BitstreamL()
: bit_idx(block_size),
bits(),
sz(0)
{}

BitstreamL::BitstreamL(std::initializer_list<bool> args)
: BitstreamL()
{
    *this << args;
}

BitstreamL& BitstreamL::operator<<(bool val)
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

BitstreamL& BitstreamL::operator<<(BlockType val)
{
    bits.push_back(val);
    sz += block_size;
    return *this;
}

BitstreamL& BitstreamL::operator<<(std::initializer_list<bool> args)
{
    // todo: perf improve: use operator<<(bool) as long as the block is not aligned
    //          then use operator<<(BlockType) if enough bits are available
    for (auto& bit : args)
        *this << bit;
    return *this;
}

std::ostream& operator<<(std::ostream& out, const BitstreamL& bitstream)
{
    for (const auto& bit : bitstream.bits)
        out.write((const char*)&bit, sizeof(bit));
    return out;
}

std::istream& operator>>(std::istream& in, BitstreamL& bitstream)
{
    BitstreamL::BlockType b = 0;
    while (in.read((char*) &b, sizeof(b)))
        bitstream << b;
    return in;
}

size_t BitstreamL::size() const
{
    return sz;
}

BitstreamL::BitView BitstreamL::operator[](unsigned int pos) {
    auto block_idx = pos / block_size;
    auto bit_idx = static_cast<uint8_t>(block_size - (pos - block_idx * block_size) - 1);

    return BitView(bits, block_idx, bit_idx);
}
