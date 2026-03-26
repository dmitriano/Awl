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
    template <class NullChecker>
    using SharedHandle = BasicSharedHandle<NullChecker, HandleDeleter, HandleDuplicator>;

    using SharedFileHandle = SharedHandle<FileNullChecker>;
    using SharedProcessHandle = SharedHandle<ProcessNullChecker>;
}
