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
            auto h_process = ::GetCurrentProcess();
            HANDLE duplicated = nullptr;

            BOOL bRes = ::DuplicateHandle(
                h_process, h, h_process, &duplicated, MAXIMUM_ALLOWED, FALSE, DUPLICATE_SAME_ACCESS);

            assert(bRes);
            static_cast<void>(bRes);

            return duplicated;
        }
    };
}
