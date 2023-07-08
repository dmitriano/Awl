/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "NonOwningHandle.h"

#include <cassert>

namespace awl::io
{
    class UniqueHandle
    {
    public:
        
        UniqueHandle()
            : m_h(NullHandleValue)
        {
        }

        UniqueHandle(HANDLE h)
            : m_h(h)
        {
        }

        UniqueHandle(const UniqueHandle& other) = delete;

        UniqueHandle(UniqueHandle&& other) noexcept
            : m_h(other.m_h)
        {
            other.m_h = NullHandleValue;
        }

        ~UniqueHandle()
        {
            Close();
        }

        UniqueHandle& operator=(const UniqueHandle& other) = delete;

        UniqueHandle& operator=(UniqueHandle&& other) noexcept
        {
            Close();

            m_h = other.m_h;

            other.m_h = NullHandleValue;

            return *this;
        }

        operator HANDLE() const
        {
            return m_h;
        }

        operator bool() const
        {
            return m_h != NullHandleValue;
        }

        void Close()
        {
            if (m_h != NullHandleValue)
            {
                int res = ::close(m_h);

                assert(res == 0);
                static_cast<void>(res);

                m_h = NullHandleValue;
            }
        }

        static bool IsNull(HANDLE h)
        {
            return h == NullHandleValue;
        }

    private:
        
        HANDLE m_h;
    };

    using UniqueFileHandle = UniqueHandle;
}
