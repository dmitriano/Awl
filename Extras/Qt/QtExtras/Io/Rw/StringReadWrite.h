/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
#pragma once

#include "Awl/Io/ReadWrite.h"
#include "Awl/Mp/TypeDescriptor.h"

#include "QtExtras/Io/Rw/ReadWrite.h"

#include <QString>

namespace awl::mp
{
    template <>
    struct type_descriptor<QString>
    {
        using inner_tuple = std::tuple<>;

        static constexpr auto name()
        {
            return type_descriptor<QByteArray>().name();
        }
    };

    static_assert(make_type_name<QString>() == make_type_name<std::string>());
}

namespace awl::io
{
    template <class Stream, class Context = FakeContext>
    inline void Read(Stream & s, QString & val, const Context & ctx = {})
    {
        QByteArray a;
        Read(s, a, ctx);

        val = QString::fromUtf8(a);
    }

    template <class Stream, class Context = FakeContext>
    inline void Write(Stream & s, const QString & val, const Context & ctx = {})
    {
        // Temporary objects are alive until the end of the full expression they're part of.
        // For a function call expression, any temporary object passed as argument to the 
        // function will be alive until the function returns.
        Write(s, val.toUtf8(), ctx);
    }
}
