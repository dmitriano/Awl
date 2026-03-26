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
            BOOL bRes = ::CloseHandle(h);

            assert(bRes);
            static_cast<void>(bRes);
        }
    };

    template <HANDLE NullHandleValue>
    using UniqueHandle = BasicUniqueHandle<NullHandleValue, UniqueHandleDeleter>;

    using UniqueFileHandle = UniqueHandle<INVALID_HANDLE_VALUE>;
    using UniqueProcessHandle = UniqueHandle<nullptr>;
}
