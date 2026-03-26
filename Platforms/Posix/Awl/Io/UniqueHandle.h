/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/BasicUniqueHandle.h"

#include <cassert>

namespace awl::io
{
    struct UniqueHandleDeleter
    {
        void operator()(HANDLE h) const
        {
            int res = ::close(h);

            assert(res == 0);
            static_cast<void>(res);
        }
    };

    using UniqueHandle = BasicUniqueHandle<NullHandleValue, UniqueHandleDeleter>;
    using UniqueFileHandle = UniqueHandle;
}
