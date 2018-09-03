#pragma once

#include <stdint.h>

namespace awl
{
    namespace crypto
    {
        uint64_t CalcCrc64(const uint8_t * data, size_t len);

        class Crc64
        {
        public:

            typedef uint64_t value_type;

            uint64_t operator()(const uint8_t * data, size_t len)
            {
                return CalcCrc64(data, len);
            }
        };
    }
}
