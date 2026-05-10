/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/HandleDeleter.h"
#include "Awl/Io/HandleDuplicator.h"
#include "Awl/Io/BasicSharedHandle.h"
#include "Awl/Io/NullGetter.h"
namespace awl::io
{
    template <class NullGetter>
    using SharedHandle = BasicSharedHandle<NullGetter, HandleDeleter, HandleDuplicator>;

    using SharedFileHandle = SharedHandle<FileNullGetter>;
    using SharedProcessHandle = SharedHandle<ProcessNullGetter>;
}
