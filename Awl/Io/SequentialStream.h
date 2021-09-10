/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <cstdint>
#include <iostream>

namespace awl 
{
    namespace io
    {
        class SequentialInputStream
        {
        public:

            virtual bool End() = 0;
                
            virtual size_t Read(uint8_t * buffer, size_t count) = 0;

            virtual ~SequentialInputStream() = default;
        };

        class SequentialOutputStream
        {
        public:

            virtual void Write(const uint8_t * buffer, size_t count) = 0;

            virtual ~SequentialOutputStream() = default;
        };
    }
}
