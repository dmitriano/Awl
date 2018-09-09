#pragma once

#include <stdint.h>
#include <array>
#include <type_traits>

namespace awl
{
    namespace crypto
    {
        //The standard library does not impose specific algorithms for calculating hashes, so we won't get portable hash codes from the standard library,
        //even for the same system, if we use a different compiler. So we need our own.

        template <size_t N> using HashValue = std::array<uint8_t, N>;

        template <size_t N>
        class BasicHash
        {
        private:

            static constexpr size_t value_size = N;

        public:

            static constexpr size_t size()
            {
                return value_size;
            }

            typedef HashValue<value_size> value_type;
        };

        template <typename T>
        constexpr std::array<std::uint8_t, sizeof(T)> to_array(T value)
        {
            std::array<std::uint8_t, sizeof(T)> result;

            for (std::size_t i{ sizeof(T) }; i != 0; --i)
            {
                result[i - 1] = static_cast<uint8_t>(value >> ((sizeof(T) - i) * 8));
            }

            return result;
        }

        template <typename T>
        constexpr T from_array(const std::array<std::uint8_t, sizeof(T)> & a)
        {
            T val = 0;

            for (uint8_t byte : a)
            {
                val = (val << 8) + byte;
            }

            return val;
        }
    }
}
