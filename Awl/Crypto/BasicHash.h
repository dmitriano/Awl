#pragma once

#include "Awl/Lang.h"

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

        class FakeHash : public BasicHash<0u>
        {
        public:

            template <class InputIt>
            constexpr value_type operator()(InputIt begin, InputIt end) const
            {
                static_cast<void>(begin);
                static_cast<void>(end);
                return {};
            }
        };

        template <typename T>
        AWL_CONSTEXPR std::array<std::uint8_t, sizeof(T)> tuple_to_array(T value)
        {
            //'= {}' is for preventing GCC warning "there is no default constructor..."
            std::array<std::uint8_t, sizeof(T)> result = {};

            for (std::size_t i{ sizeof(T) }; i != 0; --i)
            {
                result[i - 1] = static_cast<uint8_t>(value >> ((sizeof(T) - i) * 8));
            }

            return result;
        }

        template <typename T>
        AWL_CONSTEXPR T from_array(const std::array<std::uint8_t, sizeof(T)> & a)
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
