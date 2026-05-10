/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/Platform.h"

#include <cassert>
namespace awl::io
{
    struct HandleDuplicator
    {
        HANDLE operator()(HANDLE h) const
        {
            int duplicated = ::fcntl(h, F_DUPFD_CLOEXEC, 0);

            assert(duplicated != NullHandleValue);
            static_cast<void>(duplicated);

            return duplicated;
        }
    };
}
