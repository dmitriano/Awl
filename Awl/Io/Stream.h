/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/SequentialStream.h"

#include <cstddef>

namespace awl
{
    namespace io
    {
        class StreamPointer
        {
        public:

            virtual size_t GetPosition() = 0;

            virtual void Seek(std::size_t pos, bool begin = true) = 0;

            virtual void Move(std::ptrdiff_t offset) = 0;

            virtual size_t GetLength() = 0;

            virtual ~StreamPointer() = default;
        };

        class InputStream : public SequentialInputStream, public virtual StreamPointer
        {
        };

        class OutputStream : public SequentialOutputStream, public virtual StreamPointer
        {
        public:

            virtual void Flush() = 0;

            virtual void Truncate() = 0;
        };

        class IoStream : public InputStream, public OutputStream
        {
        };
    }
}
