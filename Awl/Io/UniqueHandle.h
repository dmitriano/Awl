/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/HandleDeleter.h"
#include "Awl/Io/BasicUniqueHandle.h"
#include "Awl/Io/NullHandleValues.h"

namespace awl::io
{
    template <class NullChecker>
    using UniqueHandle = BasicUniqueHandle<NullChecker, HandleDeleter>;

    using UniqueFileHandle = UniqueHandle<FileNullChecker>;
    using UniqueProcessHandle = UniqueHandle<ProcessNullChecker>;
}
