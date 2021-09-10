/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/Rw/ReadRaw.h"

#include <chrono>
#include <type_traits>

namespace awl::io
{
    //Check if nanoseconds representation is either long or long long and its size is 8, so it can be converted to int64_t.
    static_assert(std::is_arithmetic_v<std::chrono::nanoseconds::rep> && std::is_signed_v<std::chrono::nanoseconds::rep> && sizeof(std::chrono::nanoseconds::rep) == 8);

    template <class Stream, class Clock, class Duration, class Context = FakeContext>
    void Read(Stream & s, std::chrono::time_point<Clock, Duration> & val, const Context & ctx = {})
    {
        using namespace std::chrono;

        int64_t ns_count;

        Read(s, ns_count, ctx);

        val = std::chrono::time_point<Clock, Duration>(duration_cast<Duration>(nanoseconds(ns_count)));
    }

    template <class Stream, class Clock, class Duration, class Context = FakeContext>
    void Write(Stream & s, std::chrono::time_point<Clock, Duration> val, const Context & ctx = {})
    {
        using namespace std::chrono;

        const nanoseconds ns = duration_cast<nanoseconds>(val.time_since_epoch());

        const int64_t ns_count = ns.count();

        Write(s, ns_count, ctx);
    }
}
