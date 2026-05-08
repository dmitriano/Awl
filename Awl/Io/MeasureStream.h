/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/SequentialStream.h"

namespace awl
{
    namespace io
    {
        class MeasureStream : public SequentialOutputStream
        {
        public:

            void write(const uint8_t * buffer, size_t count) override
            {
                static_cast<void>(buffer);
                _pos += count;
            }

            size_t length() const
            {
                return _pos;
            }

        private:

            size_t _pos = 0;
        };
    }
}
