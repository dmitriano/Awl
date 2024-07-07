/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/Rw/ReadRaw.h"

#include <memory>
#include <type_traits>

namespace awl::io
{
    template <class Stream, typename T, class Context = FakeContext>
        requires sequential_input_stream<Stream>
    void Read(Stream& s, std::shared_ptr<T>& p, const Context & ctx = {})
    {
        bool has_value;

        Read(s, has_value, ctx);

        if (has_value)
        {
            T val;

            Read(s, val, ctx);

            p = std::make_shared<T>(val);
        }
    }

    template <class Stream, typename T, class Context = FakeContext>
        requires sequential_output_stream<Stream>
    void Write(Stream& s, const std::shared_ptr<T>& p, const Context & ctx = {})
    {
        const bool has_value = p != nullptr;

        Write(s, has_value, ctx);

        if (has_value)
        {
            Write(s, *p, ctx);
        }
    }

    template <class Stream, typename T, class Context = FakeContext>
        requires sequential_input_stream<Stream>
    void Read(Stream& s, std::unique_ptr<T>& p, const Context& ctx = {})
    {
        bool has_value;

        Read(s, has_value, ctx);

        if (has_value)
        {
            T val;

            Read(s, val, ctx);

            p = std::make_unique<T>(val);
        }
    }

    template <class Stream, typename T, class Context = FakeContext>
        requires sequential_output_stream<Stream>
    void Write(Stream& s, const std::unique_ptr<T>& p, const Context& ctx = {})
    {
        const bool has_value = p != nullptr;

        Write(s, has_value, ctx);

        if (has_value)
        {
            Write(s, *p, ctx);
        }
    }
}
