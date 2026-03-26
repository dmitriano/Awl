/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/HandleDeleter.h"
#include "Awl/Io/HandleDuplicator.h"
#include "Awl/Io/BasicSharedHandle.h"

namespace awl::io
{
    template <HANDLE NullHandleValue>
    using SharedHandle = BasicSharedHandle<NullHandleValue, HandleDeleter, HandleDuplicator>;

    using SharedFileHandle = SharedHandle<INVALID_HANDLE_VALUE>;
    using SharedProcessHandle = SharedHandle<nullptr>;
}
