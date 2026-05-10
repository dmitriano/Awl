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
    requires requires
    {
        { NullGetter::null() } noexcept -> std::convertible_to<HANDLE>;
    } &&
        std::is_nothrow_default_constructible_v<Deleter> &&
        std::is_nothrow_copy_constructible_v<Deleter> &&
        std::is_nothrow_default_constructible_v<Duplicator> &&
        std::is_nothrow_copy_constructible_v<Duplicator> &&
        std::is_nothrow_copy_assignable_v<Deleter> &&
        std::is_nothrow_copy_assignable_v<Duplicator> &&
        std::is_nothrow_invocable_v<Deleter&, HANDLE> &&
        std::invocable<const Duplicator&, HANDLE> &&
        std::convertible_to<std::invoke_result_t<const Duplicator&, HANDLE>, HANDLE>
    class BasicSharedHandle
    {
    public:

        using handle_type = HANDLE;
        using null_getter_type = NullGetter;
        using deleter_type = Deleter;
        using duplicator_type = Duplicator;

        BasicSharedHandle() noexcept
            : BasicSharedHandle(null())
        {}

        BasicSharedHandle(HANDLE h) noexcept
            : _h(h)
            , _deleter()
            , _duplicator()
        {}

        BasicSharedHandle(HANDLE h, const deleter_type& deleter, const duplicator_type& duplicator)
            noexcept
            : _h(h)
            , _deleter(deleter)
            , _duplicator(duplicator)
        {}

        BasicSharedHandle(BasicSharedHandle&& other) noexcept
            : _h(other.release())
            , _deleter(other._deleter)
            , _duplicator(other._duplicator)
        {}

        BasicSharedHandle(const BasicSharedHandle& other)
            noexcept(duplicateNoexcept)
            : _h(other.duplicate(other._h))
            , _deleter(other._deleter)
            , _duplicator(other._duplicator)
        {}

        ~BasicSharedHandle()
        {
            close();
        }

        BasicSharedHandle& operator=(BasicSharedHandle&& other) noexcept
        {
            if (this != &other)
            {
                close();

                _h = other.release();
                _deleter = other._deleter;
                _duplicator = other._duplicator;
            }

            return *this;
        }

        BasicSharedHandle& operator=(const BasicSharedHandle& other) noexcept(duplicateNoexcept)
        {
            if (this != &other)
            {
                HANDLE h = other.duplicate(other._h);

                close();

                _h = h;
                _deleter = other._deleter;
                _duplicator = other._duplicator;
            }

            return *this;
        }

        bool operator==(const BasicSharedHandle& other) const noexcept
        {
            return _h == other._h;
        }

        HANDLE get() const noexcept
        {
            return _h;
        }

        deleter_type& get_deleter() noexcept
        {
            return _deleter;
        }

        const deleter_type& get_deleter() const noexcept
        {
            return _deleter;
        }

        duplicator_type& get_duplicator() noexcept
        {
            return _duplicator;
        }

        const duplicator_type& get_duplicator() const noexcept
        {
            return _duplicator;
        }

        operator HANDLE() const noexcept
        {
            return get();
        }

        operator bool() const noexcept
        {
            return _h != null();
        }

        HANDLE release() noexcept
        {
            HANDLE h = _h;

            _h = null();

            return h;
        }

        void reset(HANDLE h = null()) noexcept
        {
            if (_h == h)
            {
                return;
            }

            close();

            _h = h;
        }

        void close() noexcept
        {
            if (_h != null())
            {
                HANDLE h = release();
                _deleter(h);
            }
        }

    private:

        static constexpr bool duplicateNoexcept = std::is_nothrow_invocable_v<const duplicator_type&, HANDLE>;

        static HANDLE null() noexcept
        {
            return null_getter_type::null();
        }

        HANDLE _h;
        deleter_type _deleter;
        duplicator_type _duplicator;

        HANDLE duplicate(HANDLE h) const noexcept(duplicateNoexcept)
        {
            if (h == null())
            {
                return null();
            }

            return _duplicator(h);
        }
    };
}
