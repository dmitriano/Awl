/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/Rw/ReadRaw.h"

#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <type_traits>

namespace awl::io
{
    namespace detail
    {
        template <class T>
        using ArithmeticBytes = std::array<std::byte, sizeof(T)>;

        template <class T>
        constexpr ArithmeticBytes<T> ToLittleEndianBytes(T value)
        {
            auto bytes = std::bit_cast<ArithmeticBytes<T>>(value);

            if constexpr (std::endian::native == std::endian::little)
            {
                return bytes;
            }
            else
            {
                ArithmeticBytes<T> reversed{};

                std::reverse_copy(bytes.begin(), bytes.end(), reversed.begin());

                return reversed;
            }
        }

        template <class T>
        constexpr T FromLittleEndianBytes(const ArithmeticBytes<T>& bytes)
        {
            if constexpr (std::endian::native == std::endian::little)
            {
                return std::bit_cast<T>(bytes);
            }
            else
            {
                ArithmeticBytes<T> reversed{};

                std::reverse_copy(bytes.begin(), bytes.end(), reversed.begin());

                return std::bit_cast<T>(reversed);
            }
        }
    }

    template <class Stream, typename T, class Context = FakeContext>
        requires (sequential_input_stream<Stream> && std::is_arithmetic_v<T> && !std::is_same<T, bool>::value)
    void Read(Stream & s, T & val, const Context & ctx = {})
    {
        static_cast<void>(ctx);

        detail::ArithmeticBytes<T> bytes{};

        ReadRaw(s, reinterpret_cast<uint8_t *>(bytes.data()), bytes.size());

        val = detail::FromLittleEndianBytes<T>(bytes);
    }

    //Scalar types are passed by value but not by const reference.
    template <class Stream, typename T, class Context = FakeContext>
        requires (sequential_output_stream<Stream> && std::is_arithmetic_v<T> && !std::is_same<T, bool>::value)
    void Write(Stream & s, T val, const Context & ctx = {})
    {
        static_cast<void>(ctx);

        auto bytes = detail::ToLittleEndianBytes(val);

        s.Write(reinterpret_cast<const uint8_t *>(bytes.data()), bytes.size());
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
