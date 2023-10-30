/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/Serializable.h"
#include "Awl/Io/ReadWrite.h"

namespace awl::io
{
    template <class T, class IStream = SequentialInputStream, class OStream = SequentialOutputStream>
    class PlainSerializable : public Serializable<IStream, OStream>
    {
    public:

        PlainSerializable(T& val) : m_val(val) {}

        void Read(IStream& s) override
        {
            awl::io::Read(s, m_val);
        }

        void Write(OStream& s) const
        {
            awl::io::Write(s, m_val);
        }

    private:

        T& m_val;
    };
}
