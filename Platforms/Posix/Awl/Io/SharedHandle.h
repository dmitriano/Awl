/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/HandleDeleter.h"
#include "Awl/Io/BasicSharedHandle.h"

namespace awl::io
{
    struct SharedHandleDuplicator
    {
        HANDLE operator()(HANDLE h) const
        {
            int duplicated = ::fcntl(h, F_DUPFD_CLOEXEC, 0);

            assert(duplicated != NullHandleValue);
            static_cast<void>(duplicated);

            return duplicated;
        }
    };

    using SharedHandle = BasicSharedHandle<NullHandleValue, HandleDeleter, SharedHandleDuplicator>;
    using SharedFileHandle = SharedHandle;
}
