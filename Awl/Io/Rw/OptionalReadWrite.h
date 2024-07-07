/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/Rw/ReadRaw.h"

#include <optional>
#include <type_traits>

namespace awl::io
{
    template <class Stream, typename T, class Context = FakeContext>
        requires sequential_input_stream<Stream>
    void Read(Stream & s, std::optional<T>& opt_val, const Context & ctx = {})
    {
        bool has_value;

        Read(s, has_value, ctx);

        if (has_value)
        {
            T val;

            Read(s, val, ctx);

            opt_val = std::move(val);
        }
    }

    template <class Stream, typename T, class Context = FakeContext>
        requires sequential_output_stream<Stream>
    void Write(Stream & s, const std::optional<T>& opt_val, const Context & ctx = {})
    {
        const bool has_value = opt_val.has_value();

        Write(s, has_value, ctx);

        if (has_value)
        {
            Write(s, opt_val.value(), ctx);
        }
    }
}
