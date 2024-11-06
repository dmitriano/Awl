/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/Serializable.h"
#include "Awl/Io/Vts.h"
#include "Awl/Mp/Mp.h"

namespace awl::io
{
    template <class T, class V, class IStream = SequentialInputStream, class OStream = SequentialOutputStream, bool atomic = true>
    class VersionTolerantSerializable : public Serializable<IStream, OStream>
    {
    private:

        using Reader = awl::io::Reader<V, IStream>;
        using Writer = awl::io::Writer<V, OStream>;

    public:

        VersionTolerantSerializable(T& val) : m_val(val) {}

        void Read(IStream& in) override
        {
            if constexpr (atomic)
            {
                // Initialize newly added fields with default values.
                T val = {};

                Read(in, val);

                //If Read throws m_val does not change.
                m_val = std::move(val);
            }
            else
            {
                Read(in, m_val);
            }
        }

        void Write(OStream& out) const override
        {
            Writer ctx;

            ctx.WriteNewPrototypes(out);
            ctx.WriteV(out, m_val);
        }

    protected:

        void Read(IStream& in, T& val)
        {
            Reader ctx;
            ctx.ReadOldPrototypes(in);

            ctx.ReadV(in, val);
        }

        T& m_val;
    };
}
