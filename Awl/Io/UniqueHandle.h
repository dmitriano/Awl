/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/HandleDeleter.h"
#include "Awl/Io/BasicUniqueHandle.h"
#include "Awl/Io/NullGetter.h"
namespace awl::io
{
    template <class NullGetter>
    using UniqueHandle = BasicUniqueHandle<NullGetter, HandleDeleter>;

    using UniqueFileHandle = UniqueHandle<FileNullGetter>;
    using UniqueProcessHandle = UniqueHandle<ProcessNullGetter>;
}
