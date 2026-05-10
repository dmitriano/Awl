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
        void writeNewPrototypes(Stream & s) const
        {
            //Write type map
            typename Base::I2nMap tm = Base::TypeMapBuilder::buildI2nMap();
            write(s, tm);

            //Write std::array.
            write(s, this->newPrototypes.size());

            for (Prototype * p : this->newPrototypes)
            {
                const size_t count = p->count();
                write(s, count);

                for (size_t i = 0; i < count; ++i)
                {
                    Field f = p->field(i);
                    //Write name as string_view but read as string.
                    const size_t len = f.name.length();
                    write(s, len);
                    s.write(const_data_cast(f.name.data()), len * sizeof(char));
                    write(s, f.type);
                }
            }
        }

        //Writes the object tree and adds indices to the structures.
        template<class Struct>
        void writeV(OutputStream & s, const Struct & val) const
        {
            if constexpr (is_reflectable_v<Struct>)
            {
                const typename Base::StructIndexType index = static_cast<typename Base::StructIndexType>(Base::template StructIndex<Struct>);
                write(s, index);
            }

            if constexpr (is_tuplizable_v<Struct>)
            {
                for_each(object_as_tuple(val), [this, &s](auto& field)
                {
                    this->writeV(s, field);
                });
            }
            else
            {
                write(s, val, *this);
            }
        }
    };
}
