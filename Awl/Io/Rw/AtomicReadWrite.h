/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/Rw/ReadRaw.h"

#include <atomic>
#include <type_traits>

namespace awl::io
{
    template <class Stream, typename T, class Context = FakeContext>
        requires sequential_input_stream<Stream>
    void Read(Stream & s, std::atomic<T>& atomic_val, const Context & ctx = {})
    {
        T val;

        Read(s, val, ctx);

        atomic_val = std::move(val);
    }

    template <class Stream, typename T, class Context = FakeContext>
        requires sequential_output_stream<Stream>
    void Write(Stream & s, const std::atomic<T>& atomic_val, const Context & ctx = {})
    {
        T val = atomic_val.load();

        Write(s, val, ctx);
    }
}
