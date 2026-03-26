/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/BasicSharedHandle.h"

#include <cassert>

namespace awl::io
{
    struct SharedHandleDeleter
    {
        void operator()(HANDLE h) const
        {
            BOOL bRes = ::CloseHandle(h);

            assert(bRes);
            static_cast<void>(bRes);
        }
    };

    struct SharedHandleDuplicator
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

    template <HANDLE NullHandleValue>
    using SharedHandle = BasicSharedHandle<NullHandleValue, SharedHandleDeleter, SharedHandleDuplicator>;

    using SharedFileHandle = SharedHandle<INVALID_HANDLE_VALUE>;
    using SharedProcessHandle = SharedHandle<nullptr>;
}
