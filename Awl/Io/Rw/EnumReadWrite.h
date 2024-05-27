/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/Rw/ArithmeticReadWrite.h"

#include <type_traits>

namespace awl::io
{
    template <class Stream, typename T, class Context = FakeContext> requires std::is_enum_v<T>
    void Read(Stream & s, T& val, const Context & ctx = {})
    {
        using Int = std::underlying_type_t<T>;
        
        Int int_val;

        Read(s, int_val, ctx);

        val = static_cast<T>(int_val);
    }

    template <class Stream, typename T, class Context = FakeContext> requires std::is_enum_v<T>
    void Write(Stream & s, T val, const Context & ctx = {})
    {
        using Int = std::underlying_type_t<T>;

        const Int int_val = static_cast<Int>(val);

        Write(s, int_val, ctx);
    }
}
