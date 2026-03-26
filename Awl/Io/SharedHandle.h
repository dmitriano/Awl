/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/HandleDeleter.h"
#include "Awl/Io/HandleDuplicator.h"
#include "Awl/Io/BasicSharedHandle.h"
#include "Awl/Io/NullHandleValues.h"

namespace awl::io
{
    template <HANDLE NullHandleValue>
    using SharedHandle = BasicSharedHandle<NullHandleValue, HandleDeleter, HandleDuplicator>;

    using SharedFileHandle = SharedHandle<NullFileHandleValue>;
    using SharedProcessHandle = SharedHandle<NullProcessHandleValue>;
}
