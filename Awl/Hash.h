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
        
        class Crc64 : public BasicHash<sizeof(uint64_t)>
        {
        public:

            template <class InputIt>
            typename std::enable_if<std::is_arithmetic<typename std::iterator_traits<InputIt>::value_type>::value, value_type>::type operator()(InputIt begin, InputIt end) const
            {
                typedef typename std::iterator_traits<InputIt>::value_type T;
                
                uint64_t crc = 0;

                for (InputIt i = begin; i != end; ++i)
                {
                    if constexpr (sizeof(T) == 1)
                    {
                        Calc(crc, static_cast<uint8_t>(*i));
                    }
                    else
                    {
                        auto bytes = to_array(*i);

                        for (size_t j = 0; j < bytes.size(); ++j)
                        {
                            Calc(crc, static_cast<uint8_t>(bytes[j]));
                        }
                    }
                }

                return to_array(crc);
            }

        private:

            static const uint64_t crc64_tab[256];

            static void Calc(uint64_t & crc, uint8_t byte)
            {
                crc = crc64_tab[(uint8_t)crc ^ byte] ^ (crc >> 8);
            };

            template <typename T>
            static std::array<std::uint8_t, sizeof(T)> to_array(T value)
            {
                std::array<std::uint8_t, sizeof(T)> result;

                for (std::size_t i{ sizeof(T) }; i; --i)
                {
                    result[i - 1] = static_cast<uint8_t>(value >> ((sizeof(T) - i) * 8));
                }

                return result;
            }
        };
    }
}
