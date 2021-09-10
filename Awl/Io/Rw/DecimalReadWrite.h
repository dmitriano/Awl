/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/Rw/ArithmeticReadWrite.h"

#include "Awl/Decimal.h"

namespace awl::io
{
    template <class Stream, class Context = FakeContext>
    inline void Read(Stream & s, awl::decimal& d, const Context & ctx = {})
    {
        uint64_t val;

        Read(s, val, ctx);

        d = awl::decimal::from_int(val);
    }

    template <class Stream, class Context = FakeContext>
    inline void Write(Stream & s, const awl::decimal& d, const Context & ctx = {})
    {
        Write(s, d.to_int(), ctx);
    }
}
