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
    inline void CheckStringLimit(size_t actual_len, size_t expected_len)
    {
        if (actual_len > expected_len)
        {
            throw IoError(format() << _T("The length of a string exceeds the limit of " << expected_len << " bytes. Actual length: " << actual_len << "."));
        }
    }

    template <class Context>
    void CheckStringLimit(const Context& ctx, size_t string_length)
    {
        if constexpr (limited_context<Context>)
        {
            CheckStringLimit(string_length, ctx.max_length());
        }
    }

    template<
        class Stream,
        class Char,
        class Traits = std::char_traits<Char>,
        class Allocator = std::allocator<Char>,
        class Context = FakeContext
    >
        requires (sequential_input_stream<Stream> && std::is_arithmetic_v<Char>)
    void Read(Stream & s, std::basic_string<Char, Traits, Allocator> & val, const Context & ctx = {})
    {
        typename std::basic_string<Char>::size_type len;

        Read(s, len, ctx);

        const size_t string_length = len * sizeof(Char);

        CheckStringLimit(ctx, string_length);

        val.resize(len);

        //There is non-const version of data() since C++ 17.
        ReadRaw(s, reinterpret_cast<uint8_t *>(val.data()), string_length);
    }

    template<
        class Stream,
        class Char,
        class Traits = std::char_traits<Char>,
        class Allocator = std::allocator<Char>,
        class Context = FakeContext
    >
        requires (sequential_output_stream<Stream> && std::is_arithmetic_v<Char>)
    void Write(Stream & s, const std::basic_string<Char, Traits, Allocator> & val, const Context & ctx = {})
    {
        typename std::basic_string<Char>::size_type len = val.length();

        const size_t string_length = len * sizeof(Char);

        CheckStringLimit(ctx, string_length);

        Write(s, len, ctx);

        s.Write(reinterpret_cast<const uint8_t *>(val.data()), string_length);
    }
}
