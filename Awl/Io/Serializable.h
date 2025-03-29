/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/SequentialStream.h"

namespace awl::io
{
    template <class IStream = SequentialInputStream, class OStream = SequentialOutputStream>
    class Serializable
    {
    public:

        virtual void Read(IStream& in) = 0;

        virtual void Write(OStream& out) const = 0;

        virtual ~Serializable() = default;
    };
}
