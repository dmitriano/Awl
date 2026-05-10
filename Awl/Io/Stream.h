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

            virtual size_t position() const = 0;

            virtual void seek(std::size_t pos, bool begin = true) = 0;

            virtual void move(std::ptrdiff_t offset) = 0;

            virtual size_t length() const = 0;

            virtual ~StreamPointer() = default;
        };

        class InputStream : public SequentialInputStream, public virtual StreamPointer {};

        class OutputStream : public SequentialOutputStream, public virtual StreamPointer
        {
        public:

            virtual void flush() = 0;

            virtual void truncate() = 0;
        };

        class IoStream : public InputStream, public OutputStream {};
    }
}
