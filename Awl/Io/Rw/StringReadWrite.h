#pragma once

#include "Awl/Io/Rw/ReadRaw.h"

#include <string>
#include <type_traits>

namespace awl::io
{
    template<
        class Stream,
        class Char,
        class Traits = std::char_traits<Char>,
        class Allocator = std::allocator<Char>,
        class Context = FakeContext
    >
    void Read(Stream & s, std::basic_string<Char, Traits, Allocator> & val, const Context & ctx = {})
    {
        typename std::basic_string<Char>::size_type len;

        Read(s, len, ctx);

        val.resize(len);

        //There is non-const version of data() since C++ 17.
        ReadRaw(s, reinterpret_cast<uint8_t *>(val.data()), len * sizeof(Char));

        *(val.data() + len) = 0;
    }

    template<
        class Stream,
        class Char,
        class Traits = std::char_traits<Char>,
        class Allocator = std::allocator<Char>,
        class Context = FakeContext
    >
    void Write(Stream & s, const std::basic_string<Char, Traits, Allocator> & val, const Context & ctx = {})
    {
        typename std::basic_string<Char>::size_type len = val.length();

        Write(s, len, ctx);

        s.Write(reinterpret_cast<const uint8_t *>(val.data()), len * sizeof(Char));
    }
}
