#pragma once

#include <cstdint>

namespace awl 
{
    namespace io
    {
        class SequentialInputStream
        {
        public:

            virtual size_t Read(uint8_t * buffer, size_t count) = 0;

            virtual ~SequentialInputStream() = default;
        };

        class SequentialOutputStream
        {
        public:

            virtual size_t Write(const uint8_t * buffer, size_t count) = 0;

            virtual ~SequentialOutputStream() = default;
        };
    }
}
