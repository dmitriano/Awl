/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/Reader.h"
#include "Awl/Io/Writer.h"
#include "Awl/Io/MeasureStream.h"
#include "Awl/Io/VectorStream.h"
#include "Awl/Io/MeasureStream.h"

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
    void WriteV(OStream& out, const T& val)
    {
        Writer<V, OStream> ctx;

        ctx.WriteNewPrototypes(out);

        ctx.WriteV(out, val);
    }

    template <class T, class V = awl::mp::variant_from_struct<T>>
    size_t MeasureV(const T& val)
    {
        MeasureStream measure_out;

        awl::io::WriteV(measure_out, val);

        return measure_out.GetLength();
    }

    template <class From, class To>
    void CopyV(const From& from_val, To& to_val)
    {
        std::vector<uint8_t> v;

        v.reserve(MeasureV(from_val));

        {
            awl::io::VectorOutputStream out(v);

            awl::io::WriteV(out, from_val);
        }

        {
            awl::io::VectorInputStream in(v);

            awl::io::ReadV(in, to_val);
        }
    }
}
