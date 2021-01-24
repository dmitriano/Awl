#pragma once

#include "Awl/Io/Rw/ArithmeticReadWrite.h"

#include "Awl/Decimal.h"

namespace awl::io
{
    template <class Stream, class Context = FakeContext>
    inline void Read(Stream & s, awl::decimal& d, const Context & ctx = {})
    {
        int64_t man;
        uint8_t digits;

        Read(s, man, ctx);
        Read(s, digits, ctx);

        d = awl::decimal(man, digits);
    }

    template <class Stream, class Context = FakeContext>
    inline void Write(Stream & s, const awl::decimal& d, const Context & ctx = {})
    {
        Write(s, d.mantissa(), ctx);
        Write(s, d.digits(), ctx);
    }
}
