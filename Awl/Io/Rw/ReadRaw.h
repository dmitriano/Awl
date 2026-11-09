/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/IoException.h"
#include "Awl/Io/SerializationContext.h"
#include "Awl/Io/SequentialStream.h"
#include "Awl/DataCast.h"

#include <cstddef>
#include <limits>

namespace awl::io
{
    //The benefit of having Stream template parameter in all Read/Write methods is that Stream::read and Stream::write functions
    //can be called as non-virtual and even as constexpr if the final Stream type is known at compile time.

    inline size_t checkedByteCount(size_t element_count, size_t element_size)
    {
        if (element_size != 0 && element_count > std::numeric_limits<size_t>::max() / element_size)
        {
            throw IoError(_T("Serialized byte count overflow."));
        }

        return element_count * element_size;
    }

    template <class Stream, class Context = FakeContext>
        requires sequential_input_stream<Stream>
    void readRaw(Stream & s, std::byte * buffer, size_t byte_count, const Context& ctx = {})
    {
        if constexpr (limited_context<Context>)
        {
            ctx.consume(byte_count);
        }

        const size_t actually_read = s.read(buffer, byte_count);

        assert(actually_read <= byte_count);

        if (actually_read < byte_count)
        {
            throw EndOfFileException(byte_count, actually_read);
        }
    }
}
