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
    template <class V, class IStream = SequentialInputStream, class Context = FakeContext>
    class PlainReader : public BasicReader<V, IStream>, public Context
    {
    private:

        using Base = BasicReader<V, IStream>;

    public:

        using InputStream = IStream;

        using Context::Context;

        PlainReader() requires std::default_initializable<Context> = default;

        //Reads entire object tree assuming all the prototypes are equal.
        template<class Struct>
        void readV(InputStream & s, Struct & val) const
        {
            if constexpr (reflectable<Struct>)
            {
                this->readStructIndex(s, static_cast<const Context&>(*this));
            }

            if constexpr (tuplizable<Struct>)
            {
                for_each(object_as_tuple(val), [this, &s](auto& field)
                {
                    this->readV(s, field);
                });
            }
            else
            {
                read(s, val, *this);
            }
        }
    };
}
