/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/Rw/ArithmeticReadWrite.h"
#include "Awl/EnumTraits.h"
#include "Awl/StringFormat.h"

#include <type_traits>

namespace awl::io
{
    template <class Stream, typename T, class Context = FakeContext>
        requires (sequential_input_stream<Stream> && std::is_enum_v<T>)
    void readEnum(Stream& s, T& val, const Context& ctx = {})
    {
        using Int = std::underlying_type_t<T>;

        Int int_val;

        read(s, int_val, ctx);

        val = static_cast<T>(int_val);
    }

    template <class Stream, typename T, class Context = FakeContext>
        requires (sequential_output_stream<Stream> && std::is_enum_v<T>)
    void writeEnum(Stream& s, T val, const Context& ctx = {})
    {
        using Int = std::underlying_type_t<T>;

        const Int int_val = static_cast<Int>(val);

        write(s, int_val, ctx);
    }

    template <class Stream, typename T, class Context = FakeContext>
        requires (sequential_input_stream<Stream> && is_nonsequential_enum<T>)
    void read(Stream & s, T& val, const Context & ctx = {})
    {
        readEnum(s, val, ctx);
    }

    template <class Stream, typename T, class Context = FakeContext>
        requires (sequential_output_stream<Stream> && is_nonsequential_enum<T>)
    void write(Stream & s, T val, const Context & ctx = {})
    {
        writeEnum(s, val, ctx);
    }

    template <class T> requires (std::is_enum_v<T> && is_defined_v<EnumTraits<T>>)
    void validateEnum(T val)
    {
        auto int_val = enum_to_underlying(val);

        if (int_val >= EnumTraits<T>::count())
        {
            throw IoError(std::format(_T("Wrong {} enum index: {}"), fromACString(EnumTraits<T>::enum_name()), int_val));
        }
    }

    template <class Stream, typename T, class Context = FakeContext>
        requires (sequential_input_stream<Stream> && is_sequential_enum<T>)
    void read(Stream& s, T& val, const Context& ctx = {})
    {
        readEnum(s, val, ctx);

        validateEnum(val);
    }

    template <class Stream, typename T, class Context = FakeContext>
        requires (sequential_output_stream<Stream> && is_sequential_enum<T>)
    void write(Stream& s, T val, const Context& ctx = {})
    {
        validateEnum(val);

        writeEnum(s, val, ctx);
    }
}
