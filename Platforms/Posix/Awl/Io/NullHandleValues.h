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
        static constexpr HANDLE Null() noexcept
        {
            return NullHandleValue;
        }

        static constexpr bool IsNull(HANDLE h) noexcept
        {
            return h == Null();
        }
    };

    struct ProcessNullChecker
    {
        static constexpr HANDLE Null() noexcept
        {
            return NullHandleValue;
        }

        static constexpr bool IsNull(HANDLE h) noexcept
        {
            return h == Null();
        }
    };
}
