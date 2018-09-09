#pragma once

#include "Awl/Crypto/BasicHash.h"

#include <stdint.h>
#include <array>
#include <type_traits>

namespace awl
{
    namespace crypto
    {
        class Crc64 : public BasicHash<sizeof(uint64_t)>
        {
        public:

#if AWL_CPPSTD >= 17
            
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
                        const auto bytes = to_array(*i);

                        for (size_t j = 0; j < bytes.size(); ++j)
                        {
                            Calc(crc, bytes[j]);
                        }
                    }
                }

                return to_array(crc);
            }

#else

            //Works a bit faster than C++ 17 version.
            template <class InputIt>
            typename std::enable_if<sizeof(typename std::iterator_traits<InputIt>::value_type) == 1 && std::is_arithmetic<typename std::iterator_traits<InputIt>::value_type>::value, value_type>::type 
                operator()(InputIt begin, InputIt end) const
            {
                uint64_t crc = 0;

                for (InputIt i = begin; i != end; ++i)
                {
                    Calc(crc, static_cast<uint8_t>(*i));
                }

                return to_array(crc);
            }

            template <class InputIt>
            typename std::enable_if<sizeof(typename std::iterator_traits<InputIt>::value_type) >= 2 && std::is_arithmetic<typename std::iterator_traits<InputIt>::value_type>::value, value_type>::type 
                operator()(InputIt begin, InputIt end) const
            {
                uint64_t crc = 0;

                for (InputIt i = begin; i != end; ++i)
                {
                    const auto bytes = to_array(*i);

                    for (size_t j = 0; j < bytes.size(); ++j)
                    {
                        Calc(crc, bytes[j]);
                    }
                }

                return to_array(crc);
            }

#endif

        private:

            static const uint64_t crc64_tab[256];

            static void Calc(uint64_t & crc, uint8_t byte)
            {
                crc = crc64_tab[(uint8_t)crc ^ byte] ^ (crc >> 8);
            };
        };
    }
}
