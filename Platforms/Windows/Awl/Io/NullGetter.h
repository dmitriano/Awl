/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/Platform.h"

namespace awl::io
{
    struct FileNullGetter
    {
        static HANDLE null() noexcept
        {
            return INVALID_HANDLE_VALUE;
        }
    };

    struct ProcessNullGetter
    {
        static constexpr HANDLE null() noexcept
        {
            return nullptr;
        }
    };
}
