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
    void read(Stream& s, decimal<UInt, exp_len, DataTemplate>& d, const Context& ctx = {})
    {
        using Decimal = decimal<UInt, exp_len, DataTemplate>;

        typename Decimal::Rep val;

        read(s, val, ctx);

        d = Decimal::from_bits(val);
    }

    template <class Stream, typename UInt, uint8_t exp_len, template <typename, uint8_t> class DataTemplate, class Context = FakeContext>
        requires sequential_output_stream<Stream>
    void write(Stream& s, const decimal<UInt, exp_len, DataTemplate>& d, const Context& ctx = {})
    {
        write(s, d.to_bits(), ctx);
    }
}
