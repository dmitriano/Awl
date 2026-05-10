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
    void readV(IStream& in, T& val)
    {
        Reader<V, IStream> ctx;

        ctx.readOldPrototypes(in);

        ctx.readV(in, val);
    }

    template <class T, class OStream = SequentialOutputStream, class V = awl::mp::variant_from_struct<T>>
        requires sequential_output_stream<OStream>
    void writeV(OStream& out, const T& val)
    {
        Writer<V, OStream> ctx;

        ctx.writeNewPrototypes(out);

        ctx.writeV(out, val);
    }

    template <class T, class V = awl::mp::variant_from_struct<T>>
    size_t measureV(const T& val)
    {
        MeasureStream measure_out;

        awl::io::writeV(measure_out, val);

        return measure_out.length();
    }

    template <class From, class To>
    void copyV(const From& from_val, To& to_val)
    {
        std::vector<uint8_t> v;

        v.reserve(measureV(from_val));

        {
            awl::io::VectorOutputStream out(v);

            awl::io::writeV(out, from_val);
        }

        {
            awl::io::VectorInputStream in(v);

            awl::io::readV(in, to_val);
        }
    }
}
