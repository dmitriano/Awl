/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/Platform.h"

namespace awl::io
{
    struct FileNullChecker
    {
        static HANDLE Null() noexcept
        {
            return INVALID_HANDLE_VALUE;
        }

        static bool IsNull(HANDLE h) noexcept
        {
            return h == Null();
        }
    };

    struct ProcessNullChecker
    {
        static constexpr HANDLE Null() noexcept
        {
            return nullptr;
        }

        static constexpr bool IsNull(HANDLE h) noexcept
        {
            return h == Null();
        }
    };
}
