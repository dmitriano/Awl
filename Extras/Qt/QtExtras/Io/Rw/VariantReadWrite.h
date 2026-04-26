/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
#pragma once

#include "Awl/Io/ReadWrite.h"
#include "Awl/Mp/TypeDescriptor.h"

#include <QVariant>

namespace awl::mp
{
    template <>
    struct type_descriptor<QVariant>
    {
        // Supported variant types goes here.
        using inner_tuple = std::tuple<>;

        static constexpr std::string name()
        {
            return "QVariant";
        }
    };
}

namespace awl::io
{
    template <class Stream, class Context = FakeContext>
    inline void read(Stream & s, QVariant & v, const Context & ctx = {})
    {
        int type;
        read(s, type, ctx);
        
        switch (type)
        {
            case QMetaType::Bool:
            {
                bool val;
                read(s, val, ctx);
                v = val;
                break;
            }
            default:
                qFatal("Type is not supported.");
                break;
        }
    }

    template <class Stream, class Context = FakeContext>
    inline void write(Stream & s, const QVariant & v, const Context & ctx = {})
    {
        const int type = v.typeId();
        write(s, type, ctx);

        switch (v.typeId())
        {
            case QMetaType::Bool:
            {
                const bool val = v.toBool();
                write(s, val, ctx);
                break;
            }
            default:
                qFatal("Type is not supported.");
                break;
        }
    }
}
