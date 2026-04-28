/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/Rw/ReadRaw.h"

#include <array>
#include <vector>
#include <type_traits>

namespace awl::io
{
    template <class Stream, class Container, class Context = FakeContext>
        requires (sequential_input_stream<Stream> && std::is_arithmetic<typename Container::value_type>::value && !std::is_same<typename Container::value_type, bool>::value)
    void readVector(Stream & s, Container & v, const Context & ctx = {})
    {
        static_cast<void>(ctx);
        readRaw(s, mutable_data_cast(v.data()), v.size() * sizeof(typename Container::value_type));
    }

    template <class Stream, class Container, class Context = FakeContext>
        requires (sequential_output_stream<Stream> && std::is_arithmetic<typename Container::value_type>::value && !std::is_same<typename Container::value_type, bool>::value)
    void writeVector(Stream & s, const Container & v, const Context & ctx = {})
    {
        static_cast<void>(ctx);
        s.write(const_data_cast(v.data()), v.size() * sizeof(typename Container::value_type));
    }

    //vector<string>, for example.
    template <class Stream, class Container, class Context = FakeContext>
        requires (sequential_input_stream<Stream> && std::is_class<typename Container::value_type>::value)
    void readVector(Stream & s, Container & v, const Context & ctx = {})
    {
        for (auto & elem : v)
        {
            read(s, elem, ctx);
        }
    }

    template <class Stream, class Container, class Context = FakeContext>
        requires (sequential_output_stream<Stream> && std::is_class<typename Container::value_type>::value)
    void writeVector(Stream & s, const Container & v, const Context & ctx = {})
    {
        for (const auto & elem : v)
        {
            write(s, elem, ctx);
        }
    }

    template <class Stream, class Container, class Context = FakeContext>
        requires (sequential_input_stream<Stream> && std::is_same<typename Container::value_type, bool>::value)
    void readVector(Stream & s, Container & x, const Context & ctx = {})
    {
        typename Container::size_type n = x.size();

        for (typename Container::size_type i = 0; i < n;)
        {
            uint8_t aggr;

            read(s, aggr, ctx);

            for (uint8_t mask = 1; mask > 0 && i < n; ++i, mask <<= 1)
            {
                x.at(i) = (aggr & mask) != 0;
            }
        }
    }

    template <class Stream, class Container, class Context = FakeContext>
        requires (sequential_output_stream<Stream> && std::is_same<typename Container::value_type, bool>::value)
    void writeVector(Stream & s, const Container & x, const Context & ctx = {})
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

            write(s, aggr, ctx);
        }
    }

    template <class Stream, class T, class Allocator = std::allocator<T>, class Context = FakeContext>
        requires sequential_input_stream<Stream>
    void read(Stream & s, std::vector<T, Allocator> & v, const Context & ctx = {})
    {
        typename std::vector<T, Allocator>::size_type size;

        read(s, size, ctx);

        v.resize(size);

        readVector(s, v, ctx);
    }

    template <class Stream, class T, class Allocator = std::allocator<T>, class Context = FakeContext>
        requires sequential_output_stream<Stream>
    void write(Stream & s, const std::vector<T, Allocator> & v, const Context & ctx = {})
    {
        typename std::vector<T, Allocator>::size_type size = v.size();

        write(s, size, ctx);

        writeVector(s, v, ctx);
    }

    //std::array has no specialization for bool type, but we save std::array<bool, N> in the same format as std::vector<bool>.
    template <class Stream, typename T, std::size_t N, class Context = FakeContext>
        requires sequential_input_stream<Stream>
    void read(Stream & s, std::array<T, N> & v, const Context & ctx = {})
    {
        readVector(s, v, ctx);
    }

    template <class Stream, typename T, std::size_t N, class Context = FakeContext>
        requires sequential_output_stream<Stream>
    void write(Stream & s, const std::array<T, N> & v, const Context & ctx = {})
    {
        writeVector(s, v, ctx);
    }
}
