/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/Rw/ReadRaw.h"
#include "Awl/Io/IoException.h"
#include "Awl/StringFormat.h"

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
        requires (sequential_input_stream<Stream> && std::is_arithmetic_v<Char>)
    void read(Stream & s, std::basic_string<Char, Traits, Allocator> & val, const Context & ctx = {})
    {
        typename std::basic_string<Char>::size_type element_count;

        read(s, element_count, ctx);

        const size_t byte_count = checkedByteCount(element_count, sizeof(Char));

        if constexpr (limited_context<Context>)
        {
            ctx.consume(element_count);
        }

        val.resize(element_count);

        //There is non-const version of data() since C++ 17.
        readRaw(s, byte_cast(val.data()), byte_count, ctx);
    }

    template<
        class Stream,
        class Char,
        class Traits = std::char_traits<Char>,
        class Allocator = std::allocator<Char>,
        class Context = FakeContext
    >
        requires (sequential_output_stream<Stream> && std::is_arithmetic_v<Char>)
    void write(Stream & s, const std::basic_string<Char, Traits, Allocator> & val, const Context & ctx = {})
    {
        typename std::basic_string<Char>::size_type len = val.length();

        const size_t byte_count = checkedByteCount(len, sizeof(Char));

        write(s, len, ctx);

        s.write(byte_cast(val.data()), byte_count);
    }
}
