#pragma once

#include <cstdint>
#include <vector>
#include <deque>
#include <initializer_list>
#include <algorithm>
#include <ostream>
#include <istream>

// Bitstream[0] is the least significant bit (LSB)
// Bitsream[Bitstream.size()-1] is the most significant bit (MSB)
class Bitstream
{
private:
    std::deque<bool> bits;

public:
    // ctors
    Bitstream();
    Bitstream(std::initializer_list<bool> args);

    // appending bits
    Bitstream& operator<<(bool val);
    Bitstream& operator<<(std::initializer_list<bool> args);

    // streaming
    friend std::ostream& operator<<(std::ostream& out, const Bitstream& bitstream);
    friend std::istream& operator>>(std::istream& in, const Bitstream& bitstream);

    // accessing
    bool& operator[](unsigned int pos);

    // others
    size_t size() const;
};

//
// Implementation
//
Bitstream::Bitstream()
{}

Bitstream::Bitstream(std::initializer_list<bool> args)
: bits(args)
{}

Bitstream& Bitstream::operator<<(bool val)
{
    bits.push_back(val);
    return *this;
}

Bitstream& Bitstream::operator<<(std::initializer_list<bool> args)
{
    bits.insert(end(bits), begin(args), end(args));
    return *this;
}

std::ostream& operator<<(std::ostream& out, const Bitstream& bitstream)
{
    for (const auto& bit : bitstream.bits)
        out.put(bit ? 1 : 0);
    return out;
}

std::istream& operator>>(std::istream& in, Bitstream& bitstream)
{
    uint8_t b;
    while (in >> b)
        bitstream << b;
    return in;
}

bool& Bitstream::operator[](unsigned int pos) {
    if (pos < bits.size())
        return bits[pos];
    throw std::out_of_range("Bitset index out of range");
}

size_t Bitstream::size() const
{
    return bits.size();
}
