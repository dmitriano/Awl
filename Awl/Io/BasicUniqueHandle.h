/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/Platform.h"

#include <concepts>
#include <type_traits>
#include <utility>

namespace awl::io
{
    template <class NullGetter, class Deleter>
    requires requires
    {
        { NullGetter::null() } noexcept -> std::convertible_to<HANDLE>;
    } &&
        std::is_nothrow_default_constructible_v<Deleter> &&
        std::is_nothrow_copy_constructible_v<Deleter> &&
        std::is_nothrow_move_constructible_v<Deleter> &&
        std::is_nothrow_copy_assignable_v<Deleter> &&
        std::is_nothrow_invocable_v<Deleter&, HANDLE>
    class BasicUniqueHandle
    {
    public:
        using handle_type = HANDLE;
        using null_getter_type = NullGetter;
        using deleter_type = Deleter;

        BasicUniqueHandle() noexcept
            : BasicUniqueHandle(null())
        {
        }

        BasicUniqueHandle(HANDLE h) noexcept
            : m_h(h)
            , m_deleter()
        {
        }

        BasicUniqueHandle(HANDLE h, const deleter_type& deleter) noexcept
            : m_h(h)
            , m_deleter(deleter)
        {
        }

        BasicUniqueHandle(HANDLE h, deleter_type&& deleter) noexcept
            : m_h(h)
            , m_deleter(std::move(deleter))
        {
        }

        BasicUniqueHandle(const BasicUniqueHandle& other) = delete;

        BasicUniqueHandle(BasicUniqueHandle&& other) noexcept
            : m_h(null())
            , m_deleter(other.m_deleter)
        {
            m_h = other.release();
        }

        ~BasicUniqueHandle()
        {
            close();
        }

        BasicUniqueHandle& operator=(const BasicUniqueHandle& other) = delete;

        BasicUniqueHandle& operator=(BasicUniqueHandle&& other) noexcept
        {
            if (this != &other)
            {
                close();

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
            return m_h != null();
        }

        HANDLE release() noexcept
        {
            HANDLE h = m_h;

            m_h = null();

            return h;
        }

        void reset(HANDLE h = null()) noexcept
        {
            if (m_h == h)
            {
                return;
            }

            close();

            m_h = h;
        }

        void close() noexcept
        {
            if (m_h != null())
            {
                HANDLE h = release();
                m_deleter(h);
            }
        }

    private:
        static HANDLE null() noexcept
        {
            return null_getter_type::null();
        }

        HANDLE m_h;
        deleter_type m_deleter;
    };
}
