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
        HANDLE operator()() const noexcept
        {
            return INVALID_HANDLE_VALUE;
        }
    };

    struct ProcessNullGetter
    {
        constexpr HANDLE operator()() const noexcept
        {
            return nullptr;
        }
    };
}
