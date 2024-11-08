/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/Rw/ReadRaw.h"
#include "Awl/Io/IoException.h"
#include "Awl/StringFormat.h"

#include <array>
#include <vector>
#include <type_traits>

namespace awl::io
{
    template <class Stream, class Container, class Context = FakeContext>
        requires (sequential_input_stream<Stream> && std::is_arithmetic<typename Container::value_type>::value && !std::is_same<typename Container::value_type, bool>::value)
    void ReadVector(Stream & s, Container & v, const Context & ctx = {})
    {
        static_cast<void>(ctx);
        ReadRaw(s, reinterpret_cast<uint8_t *>(v.data()), v.size() * sizeof(typename Container::value_type));
    }

    template <class Stream, class Container, class Context = FakeContext>
        requires (sequential_output_stream<Stream> && std::is_arithmetic<typename Container::value_type>::value && !std::is_same<typename Container::value_type, bool>::value)
    void WriteVector(Stream & s, const Container & v, const Context & ctx = {})
    {
        static_cast<void>(ctx);
        s.Write(reinterpret_cast<const uint8_t *>(v.data()), v.size() * sizeof(typename Container::value_type));
    }

    //vector<string>, for example.
    template <class Stream, class Container, class Context = FakeContext>
        requires (sequential_input_stream<Stream> && std::is_class<typename Container::value_type>::value)
    void ReadVector(Stream & s, Container & v, const Context & ctx = {})
    {
        for (auto & elem : v)
        {
            Read(s, elem, ctx);
        }
    }

    template <class Stream, class Container, class Context = FakeContext>
        requires (sequential_output_stream<Stream> && std::is_class<typename Container::value_type>::value)
    void WriteVector(Stream & s, const Container & v, const Context & ctx = {})
    {
        for (const auto & elem : v)
        {
            Write(s, elem, ctx);
        }
    }

    template <class Stream, class Container, class Context = FakeContext>
        requires (sequential_input_stream<Stream> && std::is_same<typename Container::value_type, bool>::value)
    void ReadVector(Stream & s, Container & x, const Context & ctx = {})
    {
        typename Container::size_type n = x.size();

        for (typename Container::size_type i = 0; i < n;)
        {
            uint8_t aggr;

            Read(s, aggr, ctx);

            for (uint8_t mask = 1; mask > 0 && i < n; ++i, mask <<= 1)
            {
                x.at(i) = (aggr & mask) != 0;
            }
        }
    }

    template <class Stream, class Container, class Context = FakeContext>
        requires (sequential_output_stream<Stream> && std::is_same<typename Container::value_type, bool>::value)
    void WriteVector(Stream & s, const Container & x, const Context & ctx = {})
    {
        typename Container::size_type n = x.size();

        for (typename Container::size_type i = 0; i < n;)
        {
            uint8_t aggr = 0;

            for (uint8_t mask = 1; mask > 0 && i < n; ++i, mask <<= 1)
            {
                if (x.at(i))
                {
                    aggr |= mask;
                }
            }

            Write(s, aggr, ctx);
        }
    }

    template <class Stream, class T, class Allocator = std::allocator<T>, class Context = FakeContext>
        requires sequential_input_stream<Stream>
    void Read(Stream & s, std::vector<T, Allocator> & v, const Context & ctx = {})
    {
        typename std::vector<T, Allocator>::size_type size;

        Read(s, size, ctx);

        v.resize(size);

        ReadVector(s, v, ctx);
    }

    template <class Stream, class T, class Allocator = std::allocator<T>, class Context = FakeContext>
        requires sequential_output_stream<Stream>
    void Write(Stream & s, const std::vector<T, Allocator> & v, const Context & ctx = {})
    {
        typename std::vector<T, Allocator>::size_type size = v.size();

        Write(s, size, ctx);

        WriteVector(s, v, ctx);
    }

    //std::array has no specialization for bool type, but we save std::array<bool, N> in the same format as std::vector<bool>.
    template <class Stream, typename T, std::size_t N, class Context = FakeContext>
        requires sequential_input_stream<Stream>
    void Read(Stream & s, std::array<T, N> & v, const Context & ctx = {})
    {
        if constexpr (vts_read_context<Context, Stream, T>)
        {
            // Allow to resize std::array in future versions.
            typename std::array<T, N>::size_type size;

            Read(s, size, ctx);

            if (size > N)
            {
                throw IoError(format() << _T("Read array size ") << size << _T(" is greater than the actual size ") << N << _T("."));
            }
        }

        ReadVector(s, v, ctx);
    }

    template <class Stream, typename T, std::size_t N, class Context = FakeContext>
        requires sequential_output_stream<Stream>
    void Write(Stream & s, const std::array<T, N> & v, const Context & ctx = {})
    {
        if constexpr (vts_write_context<Context, Stream, T>)
        {
            Write(s, N, ctx);
        }

        WriteVector(s, v, ctx);
    }
}
