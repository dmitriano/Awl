/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/Rw/ReadRaw.h"

#include <type_traits>

namespace awl::io
{
    template <class Stream, typename T, class Context = FakeContext>
        requires (sequential_input_stream<Stream> && std::is_arithmetic_v<T> && !std::is_same<T, bool>::value)
    void Read(Stream & s, T & val, const Context & ctx = {})
    {
        static_cast<void>(ctx);

        const size_t size = sizeof(T);

        ReadRaw(s, reinterpret_cast<uint8_t *>(&val), size);
    }

    //Scalar types are passed by value but not by const reference.
    template <class Stream, typename T, class Context = FakeContext>
        requires (sequential_output_stream<Stream> && std::is_arithmetic_v<T> && !std::is_same<T, bool>::value)
    void Write(Stream & s, T val, const Context & ctx = {})
    {
        static_cast<void>(ctx);
        
        const size_t size = sizeof(T);

        s.Write(reinterpret_cast<const uint8_t *>(&val), size);
    }

    //sizeof(bool) is implementation-defined and it is not required to be 1.

    template <class Stream, class Context = FakeContext>
        requires sequential_input_stream<Stream>
    void Read(Stream & s, bool & b, const Context & ctx = {})
    {
        uint8_t val;

        Read(s, val, ctx);

        b = val != 0;
    }

    template <class Stream, class Context = FakeContext>
        requires sequential_output_stream<Stream>
    void Write(Stream & s, bool b, const Context & ctx = {})
    {
        uint8_t val = b ? 1 : 0;

        Write(s, val, ctx);
    }
}
