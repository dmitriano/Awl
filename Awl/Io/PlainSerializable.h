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

        PlainSerializable(T& val) : m_val(val) {}

        void Read(IStream& s) override
        {
            if constexpr (atomic)
            {
                T val;

                awl::io::Read(s, val);

                //If Read throws m_val does not change.
                m_val = std::move(val);
            }
            else
            {
                awl::io::Read(s, m_val);
            }
        }

        void Write(OStream& s) const override
        {
            awl::io::Write(s, m_val);
        }

    private:

        T& m_val;
    };
}
