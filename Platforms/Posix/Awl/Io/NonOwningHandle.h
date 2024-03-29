/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/Platform.h"

namespace awl::io
{
    class NonOwningHandle
    {
    public:
        NonOwningHandle()
            : m_h(NullHandleValue)
        {
        }

        NonOwningHandle(HANDLE h)
            : m_h(h)
        {
        }

        NonOwningHandle(const NonOwningHandle&) = default;

        NonOwningHandle(NonOwningHandle&&) = default;

        //NonOwningHandle & operator = (HANDLE h)
        //{
        //    m_h = h;
        //}

        operator HANDLE() const
        {
            return m_h;
        }

        operator bool() const
        {
            return m_h != NullHandleValue;
        }

        static bool IsNull(HANDLE h)
        {
            return h == NullHandleValue;
        }

    private:

        HANDLE m_h;
    };

    using NonOwningFileHandle = NonOwningHandle;
}
