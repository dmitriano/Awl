/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/PrototypeContainer.h"
#include "Awl/Io/ReadWrite.h"
#include "Awl/Io/SequentialStream.h"
#include "Awl/Io/ReadWrite.h"

#include <cassert>

namespace awl::io
{
    template <class V, class OStream = SequentialOutputStream>
    class Writer : public PrototypeContainer<V>
    {
    private:

        using Base = PrototypeContainer<V>;

    public:

        using OutputStream = OStream;

        template <class Stream>
        void WriteNewPrototypes(Stream & s) const
        {
            //Write type map
            typename Base::I2nMap tm = Base::TypeMapBuilder::BuildI2nMap();
            Write(s, tm);

            //Write std::array.
            Write(s, this->newPrototypes.size());

            for (Prototype * p : this->newPrototypes)
            {
                const size_t count = p->GetCount();
                Write(s, count);

                for (size_t i = 0; i < count; ++i)
                {
                    Field f = p->GetField(i);
                    //Write name as string_view but read as string.
                    const size_t len = f.name.length();
                    Write(s, len);
                    s.Write(reinterpret_cast<const uint8_t *>(f.name.data()), len * sizeof(char));
                    Write(s, f.type);
                }
            }
        }

        //Writes the object tree and adds indices to the structures.
        template<class Struct>
        void WriteV(OutputStream & s, const Struct & val) const
        {
            if constexpr (is_stringizable_v<Struct>)
            {
                const typename Base::StructIndexType index = static_cast<typename Base::StructIndexType>(Base::template StructIndex<Struct>);
                Write(s, index);
            }

            if constexpr (is_tuplizable_v<Struct>)
            {
                for_each(object_as_tuple(val), [this, &s](auto& field)
                {
                    this->WriteV(s, field);
                });
            }
            else
            {
                Write(s, val, *this);
            }
        }
    };
}
