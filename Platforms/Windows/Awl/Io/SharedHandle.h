#pragma once

#include "Awl/Io/Platform.h"

#include <assert.h>

namespace awl::io
{
    template <HANDLE NullHandleValue>
    class SharedHandle
    {
    public:
        SharedHandle()
            : m_h(NullHandleValue)
        {
        }

        SharedHandle(HANDLE h)
            : m_h(h)
        {
        }

        SharedHandle(const SharedHandle& other)
        {
            Duplicate(other.m_h);
        }

        SharedHandle(SharedHandle&& other)
            : m_h(other.m_h)
        {
            other.m_h = NullHandleValue;
        }

        ~SharedHandle()
        {
            Close();
        }

        SharedHandle& operator=(const SharedHandle& other)
        {
            Close();

            Duplicate(other.m_h);

            return *this;
        }

        SharedHandle& operator=(SharedHandle&& other)
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

        void Duplicate(HANDLE h)
        {
            auto h_process = ::GetCurrentProcess();

            BOOL bRes = ::DuplicateHandle(
                h_process, h, h_process, &m_h, MAXIMUM_ALLOWED, FALSE, DUPLICATE_SAME_ACCESS);

            assert(bRes);
            static_cast<void>(bRes);
        }
    };

    typedef SharedHandle<INVALID_HANDLE_VALUE> SharedFileHandle;
}
