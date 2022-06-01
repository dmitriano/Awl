/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/Rw/ArithmeticReadWrite.h"

#include "Awl/Decimal.h"

namespace awl::io
{
    template <class Stream, typename UInt, uint8_t exp_len, class Context = FakeContext>
    void Read(Stream & s, decimal<UInt, exp_len>& d, const Context & ctx = {})
    {
        uint64_t val;

        Read(s, val, ctx);

        d = decimal<UInt, exp_len>::from_bits(val);
    }

    template <class Stream, typename UInt, uint8_t exp_len, class Context = FakeContext>
    void Write(Stream & s, const decimal<UInt, exp_len>& d, const Context & ctx = {})
    {
        Write(s, d.to_bits(), ctx);
    }
}
