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
    template <HANDLE NullHandleValue>
    using UniqueHandle = BasicUniqueHandle<NullHandleValue, HandleDeleter>;

    using UniqueFileHandle = UniqueHandle<NullFileHandleValue>;
    using UniqueProcessHandle = UniqueHandle<NullProcessHandleValue>;
}
