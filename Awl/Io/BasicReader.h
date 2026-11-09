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
    template <class V, class IStream = SequentialInputStream>
    class BasicReader : public PrototypeContainer<V>
    {
    private:

        using Base = PrototypeContainer<V>;

    public:

        using InputStream = IStream;

    protected:

        template <class Context = FakeContext>
        typename Base::StructIndexType readStructIndex(InputStream & s, const Context& ctx = {}) const
        {
            typename Base::StructIndexType index;
            read(s, index, ctx);
            return index;
        }
    };
}
