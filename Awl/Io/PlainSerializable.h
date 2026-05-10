/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/Serializable.h"
#include "Awl/Io/ReadWrite.h"

namespace awl::io
{
    template <class T, class IStream = SequentialInputStream, class OStream = SequentialOutputStream, bool atomic = true>
    class PlainSerializable : public Serializable<IStream, OStream>
    {
    public:

        PlainSerializable(T& val) : _val(val) {}

        void read(IStream& s) override
        {
            if constexpr (atomic)
            {
                // There can't be newly added fields so we leave val uninitialized.
                // All the fields should be read.
                T val;

                awl::io::read(s, val);

                //If Read throws _val does not change.
                _val = std::move(val);
            }
            else
            {
                awl::io::read(s, _val);
            }
        }

        void write(OStream& s) const override
        {
            awl::io::write(s, _val);
        }

    private:

        T& _val;
    };
}
