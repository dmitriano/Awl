/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/Rw/ArithmeticReadWrite.h"
#include "Awl/EnumTraits.h"

#include <type_traits>

namespace awl::io
{
    template <class Stream, typename T, class Context = FakeContext> requires std::is_enum_v<T>
    void ReadEnum(Stream& s, T& val, const Context& ctx = {})
    {
        using Int = std::underlying_type_t<T>;

        Int int_val;

        Read(s, int_val, ctx);

        val = static_cast<T>(int_val);
    }

    template <class Stream, typename T, class Context = FakeContext> requires std::is_enum_v<T>
    void WriteEnum(Stream& s, T val, const Context& ctx = {})
    {
        using Int = std::underlying_type_t<T>;

        const Int int_val = static_cast<Int>(val);

        Write(s, int_val, ctx);
    }

    template <class Stream, typename T, class Context = FakeContext> requires std::is_enum_v<T> && !is_defined_v<EnumTraits<T>>
    void Read(Stream & s, T& val, const Context & ctx = {})
    {
        ReadEnum(s, val, ctx);
    }

    template <class Stream, typename T, class Context = FakeContext> requires std::is_enum_v<T> && !is_defined_v<EnumTraits<T>>
    void Write(Stream & s, T val, const Context & ctx = {})
    {
        WriteEnum(s, val, ctx);
    }

    template <class Stream, typename T, class Context = FakeContext> requires std::is_enum_v<T> && is_defined_v<EnumTraits<T>>
    void Read(Stream& s, T& val, const Context& ctx = {})
    {
        ReadEnum(s, val, ctx);

        validate_enum(val);
    }

    template <class Stream, typename T, class Context = FakeContext> requires std::is_enum_v<T> && is_defined_v<EnumTraits<T>>
    void Write(Stream& s, T val, const Context& ctx = {})
    {
        validate_enum(val);

        WriteEnum(s, val, ctx);
    }
}
