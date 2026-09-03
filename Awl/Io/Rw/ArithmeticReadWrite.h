/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/Rw/ReadRaw.h"
#include "Awl/Int2Array.h"

#include <cstddef>
#include <type_traits>

namespace awl::io
{
    template <class Stream, size_t N>
        requires sequential_input_stream<Stream>
    constexpr void readBuffer(Stream& s, std::array<std::byte, N>& a)
    {
        readRaw(s, a.data(), a.size());
    }

    template <class Stream, size_t N>
        requires sequential_output_stream<Stream>
    constexpr void writeBuffer(Stream& s, const std::array<std::byte, N>& a)
    {
        s.write(a.data(), a.size());
    }

    template <class Stream, typename T, class Context = FakeContext>
        requires (sequential_input_stream<Stream> && std::is_arithmetic_v<T> && !std::is_same<T, bool>::value)
    void read(Stream & s, T & val, const Context & ctx = {})
    {
        static_cast<void>(ctx);

        std::array<std::byte, sizeof(T)> a;

        readBuffer(s, a);

        val = from_buffer<T>(a);
    }

    //Scalar types are passed by value but not by const reference.
    template <class Stream, typename T, class Context = FakeContext>
        requires (sequential_output_stream<Stream> && std::is_arithmetic_v<T> && !std::is_same<T, bool>::value)
    void write(Stream & s, T val, const Context & ctx = {})
    {
        static_cast<void>(ctx);
        
        writeBuffer(s, to_buffer(val));
    }

    //sizeof(bool) is implementation-defined and it is not required to be 1.

    template <class Stream, class Context = FakeContext>
        requires sequential_input_stream<Stream>
    void read(Stream & s, std::byte & val, const Context & ctx = {})
    {
        static_cast<void>(ctx);

        readRaw(s, &val, 1);
    }

    template <class Stream, class Context = FakeContext>
        requires sequential_output_stream<Stream>
    void write(Stream & s, std::byte val, const Context & ctx = {})
    {
        static_cast<void>(ctx);

        s.write(&val, 1);
    }

    template <class Stream, class Context = FakeContext>
        requires sequential_input_stream<Stream>
    void read(Stream & s, bool & b, const Context & ctx = {})
    {
        std::byte val;

        read(s, val, ctx);

        b = val != std::byte{};
    }

    template <class Stream, class Context = FakeContext>
        requires sequential_output_stream<Stream>
    void write(Stream & s, bool b, const Context & ctx = {})
    {
        std::byte val = b ? std::byte{1} : std::byte{};

        write(s, val, ctx);
    }
}
