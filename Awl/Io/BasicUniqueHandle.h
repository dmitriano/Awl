/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/Platform.h"

#include <type_traits>
#include <utility>

namespace awl::io
{
    template <HANDLE NullHandleValue, class Deleter>
    class BasicUniqueHandle
    {
    public:
        using handle_type = HANDLE;
        using deleter_type = Deleter;

        BasicUniqueHandle() noexcept(std::is_nothrow_default_constructible_v<deleter_type>)
            : BasicUniqueHandle(NullHandleValue)
        {
        }

        BasicUniqueHandle(HANDLE h) noexcept(std::is_nothrow_default_constructible_v<deleter_type>)
            : m_h(h)
            , m_deleter()
        {
        }

        BasicUniqueHandle(HANDLE h, const deleter_type& deleter) noexcept(std::is_nothrow_copy_constructible_v<deleter_type>)
            : m_h(h)
            , m_deleter(deleter)
        {
        }

        BasicUniqueHandle(HANDLE h, deleter_type&& deleter) noexcept(std::is_nothrow_move_constructible_v<deleter_type>)
            : m_h(h)
            , m_deleter(std::move(deleter))
        {
        }

        BasicUniqueHandle(const BasicUniqueHandle& other) = delete;

        BasicUniqueHandle(BasicUniqueHandle&& other) noexcept(std::is_nothrow_copy_constructible_v<deleter_type>)
            : m_h(NullHandleValue)
            , m_deleter(other.m_deleter)
        {
            m_h = other.release();
        }

        ~BasicUniqueHandle()
        {
            Close();
        }

        BasicUniqueHandle& operator=(const BasicUniqueHandle& other) = delete;

        BasicUniqueHandle& operator=(BasicUniqueHandle&& other) noexcept(
            std::is_nothrow_copy_assignable_v<deleter_type> &&
            noexcept(std::declval<deleter_type&>()(std::declval<HANDLE>())))
        {
            if (this != &other)
            {
                Close();

                m_deleter = other.m_deleter;
                m_h = other.release();
            }

            return *this;
        }

        bool operator==(const BasicUniqueHandle& other) const noexcept
        {
            return m_h == other.m_h;
        }

        HANDLE get() const noexcept
        {
            return m_h;
        }

        deleter_type& get_deleter() noexcept
        {
            return m_deleter;
        }

        const deleter_type& get_deleter() const noexcept
        {
            return m_deleter;
        }

        operator HANDLE() const noexcept
        {
            return get();
        }

        operator bool() const noexcept
        {
            return !IsNull(m_h);
        }

        HANDLE release() noexcept
        {
            HANDLE h = m_h;

            m_h = NullHandleValue;

            return h;
        }

        void reset(HANDLE h = NullHandleValue) noexcept(noexcept(std::declval<deleter_type&>()(std::declval<HANDLE>())))
        {
            if (m_h == h)
            {
                return;
            }

            Close();

            m_h = h;
        }

        void Close() noexcept(noexcept(std::declval<deleter_type&>()(std::declval<HANDLE>())))
        {
            if (!IsNull(m_h))
            {
                HANDLE h = release();
                m_deleter(h);
            }
        }

        static constexpr bool IsNull(HANDLE h) noexcept
        {
            return h == NullHandleValue;
        }

    private:
        HANDLE m_h;
        deleter_type m_deleter;
    };
}
