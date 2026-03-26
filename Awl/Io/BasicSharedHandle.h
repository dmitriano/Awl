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
    template <class NullGetter, class Deleter, class Duplicator>
    requires std::invocable<NullGetter> &&
        std::convertible_to<std::invoke_result_t<NullGetter>, HANDLE> &&
        std::is_nothrow_default_constructible_v<Deleter> &&
        std::is_nothrow_default_constructible_v<Duplicator> &&
        std::is_nothrow_copy_assignable_v<Deleter> &&
        std::is_nothrow_copy_assignable_v<Duplicator> &&
        std::invocable<Deleter&, HANDLE> &&
        std::invocable<const Duplicator&, HANDLE> &&
        std::convertible_to<std::invoke_result_t<const Duplicator&, HANDLE>, HANDLE>
    class BasicSharedHandle
    {
    public:
        using handle_type = HANDLE;
        using null_getter_type = NullGetter;
        using deleter_type = Deleter;
        using duplicator_type = Duplicator;

        BasicSharedHandle() noexcept(noexcept(null()))
            : BasicSharedHandle(null())
        {
        }

        BasicSharedHandle(HANDLE h) noexcept
            : m_h(h)
            , m_deleter()
            , m_duplicator()
        {
        }

        BasicSharedHandle(HANDLE h, const deleter_type& deleter, const duplicator_type& duplicator)
            noexcept(std::is_nothrow_copy_constructible_v<deleter_type> &&
                std::is_nothrow_copy_constructible_v<duplicator_type>)
            : m_h(h)
            , m_deleter(deleter)
            , m_duplicator(duplicator)
        {
        }

        BasicSharedHandle(BasicSharedHandle&& other) noexcept(
            std::is_nothrow_copy_constructible_v<deleter_type> &&
            std::is_nothrow_copy_constructible_v<duplicator_type>)
            : m_h(other.release())
            , m_deleter(other.m_deleter)
            , m_duplicator(other.m_duplicator)
        {
        }

        BasicSharedHandle(const BasicSharedHandle& other)
            noexcept(std::is_nothrow_copy_constructible_v<deleter_type> &&
                std::is_nothrow_copy_constructible_v<duplicator_type> &&
                noexcept(std::declval<const duplicator_type&>()(std::declval<HANDLE>())))
            : m_h(other.Duplicate(other.m_h))
            , m_deleter(other.m_deleter)
            , m_duplicator(other.m_duplicator)
        {
        }

        ~BasicSharedHandle()
        {
            close();
        }

        BasicSharedHandle& operator=(BasicSharedHandle&& other) noexcept(noexcept(std::declval<deleter_type&>()(std::declval<HANDLE>())))
        {
            if (this != &other)
            {
                close();

                m_h = other.release();
                m_deleter = other.m_deleter;
                m_duplicator = other.m_duplicator;
            }

            return *this;
        }

        BasicSharedHandle& operator=(const BasicSharedHandle& other) noexcept(
            noexcept(std::declval<const duplicator_type&>()(std::declval<HANDLE>())) &&
            noexcept(std::declval<deleter_type&>()(std::declval<HANDLE>())))
        {
            if (this != &other)
            {
                HANDLE h = other.Duplicate(other.m_h);

                close();

                m_h = h;
                m_deleter = other.m_deleter;
                m_duplicator = other.m_duplicator;
            }

            return *this;
        }

        bool operator==(const BasicSharedHandle& other) const noexcept
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

        duplicator_type& get_duplicator() noexcept
        {
            return m_duplicator;
        }

        const duplicator_type& get_duplicator() const noexcept
        {
            return m_duplicator;
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

        void reset(HANDLE h = null()) noexcept(noexcept(std::declval<deleter_type&>()(std::declval<HANDLE>())))
        {
            if (m_h == h)
            {
                return;
            }

            close();

            m_h = h;
        }

        void close() noexcept(noexcept(std::declval<deleter_type&>()(std::declval<HANDLE>())))
        {
            if (m_h != null())
            {
                HANDLE h = release();
                m_deleter(h);
            }
        }

    private:
        static HANDLE null() noexcept(noexcept(null_getter_type{}()))
        {
            return null_getter_type{}();
        }

        HANDLE m_h;
        deleter_type m_deleter;
        duplicator_type m_duplicator;

        HANDLE Duplicate(HANDLE h) const noexcept(noexcept(std::declval<const duplicator_type&>()(std::declval<HANDLE>())))
        {
            if (h == null())
            {
                return null();
            }

            return m_duplicator(h);
        }
    };
}
