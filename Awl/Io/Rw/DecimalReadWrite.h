/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/Rw/ArithmeticReadWrite.h"

#include "Awl/Decimal.h"

namespace awl::io
{
    template <class Stream, typename UInt, uint8_t exp_len, template <typename, uint8_t> class DataTemplate, class Context = FakeContext>
        requires sequential_input_stream<Stream>
    void Read(Stream& s, decimal<UInt, exp_len, DataTemplate>& d, const Context& ctx = {})
    {
        using Decimal = decimal<UInt, exp_len, DataTemplate>;

        typename Decimal::Rep val;

        Read(s, val, ctx);

        d = Decimal::from_bits(val);
    }

    template <class Stream, typename UInt, uint8_t exp_len, template <typename, uint8_t> class DataTemplate, class Context = FakeContext>
        requires sequential_output_stream<Stream>
    void Write(Stream& s, const decimal<UInt, exp_len, DataTemplate>& d, const Context& ctx = {})
    {
        Write(s, d.to_bits(), ctx);
    }
}
