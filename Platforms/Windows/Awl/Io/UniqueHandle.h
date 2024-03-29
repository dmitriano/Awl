/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "NonOwningHandle.h"

#include <assert.h>

namespace awl::io
{
    template <HANDLE NullHandleValue>
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

        // defaulting comparison operators requires at least '/std:c++20'
        // bool operator == (const UniqueHandle& other) const = default;

        bool operator == (const UniqueHandle& other) const
        {
            return m_h == other.m_h;
        }

        bool operator != (const UniqueHandle& other) const
        {
            return !operator == (other);
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
                BOOL bRes = ::CloseHandle(m_h);

                assert(bRes);
                static_cast<void>(bRes);

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

    using UniqueFileHandle = UniqueHandle<INVALID_HANDLE_VALUE>;
}
