/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/BasicReader.h"
#include "Awl/Io/SequentialStream.h"
#include "Awl/Reflection.h"
#include "Awl/TupleHelpers.h"

namespace awl::io
{
    template <class V, class IStream = SequentialInputStream>
    class PlainReader : public BasicReader<V, IStream>
    {
    private:

        using Base = BasicReader<V, IStream>;

    public:

        using InputStream = IStream;

        //Reads entire object tree assuming all the prototypes are equal.
        template<class Struct>
        void ReadV(InputStream & s, Struct & val) const
        {
            if constexpr (is_reflectable_v<Struct>)
            {
                this->ReadStructIndex(s);
            }

            if constexpr (tuplizable<Struct>)
            {
                for_each(object_as_tuple(val), [this, &s](auto& field)
                {
                    this->ReadV(s, field);
                });
            }
            else
            {
                Read(s, val, *this);
            }
        }
    };
}
