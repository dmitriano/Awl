/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
#pragma once

#include "Awl/Io/ReadWrite.h"
#include "Awl/Mp/TypeDescriptor.h"

#include <QByteArray>
#include <limits>

namespace awl::mp
{
    template <>
    struct type_descriptor<QByteArray>
    {
        using inner_tuple = std::tuple<>;

        static constexpr auto name()
        {
            return type_descriptor<std::vector<int8_t>>().name();
        }
    };
}

namespace awl::io
{
    template <class Stream, class Context = FakeContext>
    inline void read(Stream & s, QByteArray& val, const Context & ctx = {})
    {
        size_t std_size;
        read(s, std_size, ctx);

        if (std_size > static_cast<size_t>(std::numeric_limits<int>::max()))
        {
            throw awl::io::IoError(_T("QByteArray size exceeds the supported range."));
        }

        if constexpr (limited_context<Context>)
        {
            ctx.consume(std_size);
        }

        val.resize(static_cast<int>(std_size));
        readRaw(s, byte_cast(val.data()), std_size, ctx);
    }

    template <class Stream, class Context = FakeContext>
    inline void write(Stream & s, const QByteArray& val, const Context & ctx = {})
    {
        const size_t std_size = static_cast<size_t>(val.size());
        write(s, std_size, ctx);

        s.write(byte_cast(val.data()), std_size);
    }
}
