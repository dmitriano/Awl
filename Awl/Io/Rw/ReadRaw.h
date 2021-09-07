#pragma once

#include "Awl/Io/IoException.h"
#include "Awl/Io/Rw/FakeContext.h"

namespace awl::io
{
    //The benefit of having Stream template parameter in all Read/Write methods is that Stream::Read and Stream::Write functions
    //can be called as non-virtual and even as constexpr if the final Stream type is known at compile time.

    template <class Stream>
    void ReadRaw(Stream & s, uint8_t * buffer, size_t count)
    {
        const size_t actually_read = s.Read(buffer, count);

        assert(actually_read <= count);

        if (actually_read < count)
        {
            throw EndOfFileException(count, actually_read);
        }
    }
}
