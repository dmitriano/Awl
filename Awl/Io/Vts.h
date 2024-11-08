/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/Reader.h"
#include "Awl/Io/Writer.h"

namespace awl::io
{
    template <class T, class IStream = SequentialInputStream, class V = awl::mp::variant_from_struct<T>>
        requires sequential_input_stream<IStream>
    void ReadV(IStream& in, T& val)
    {
        Reader<V, IStream> ctx;

        ctx.ReadOldPrototypes(in);

        ctx.ReadV(in, val);
    }

    template <class T, class OStream = SequentialOutputStream, class V = awl::mp::variant_from_struct<T>>
        requires sequential_output_stream<OStream>
    void WriteV(OStream& out, T& val)
    {
        Writer<V, OStream> ctx;

        ctx.WriteNewPrototypes(out);

        ctx.WriteV(out, val);
    }
}
