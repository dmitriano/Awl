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

            void Write(const uint8_t * buffer, size_t count) override
            {
                static_cast<void>(buffer);
                m_pos += count;
            }

            size_t GetLength() const
            {
                return m_pos;
            }

        private:

            size_t m_pos = 0;
        };
    }
}
