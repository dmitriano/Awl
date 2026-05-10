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
        {}

        BasicUniqueHandle(HANDLE h) noexcept
            : _h(h)
            , _deleter()
        {}

        BasicUniqueHandle(HANDLE h, const deleter_type& deleter) noexcept
            : _h(h)
            , _deleter(deleter)
        {}

        BasicUniqueHandle(HANDLE h, deleter_type&& deleter) noexcept
            : _h(h)
            , _deleter(std::move(deleter))
        {}

        BasicUniqueHandle(const BasicUniqueHandle& other) = delete;

        BasicUniqueHandle(BasicUniqueHandle&& other) noexcept
            : _h(null())
            , _deleter(other._deleter)
        {
            _h = other.release();
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

                _deleter = other._deleter;
                _h = other.release();
            }

            return *this;
        }

        bool operator==(const BasicUniqueHandle& other) const noexcept
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

        static HANDLE null() noexcept
        {
            return null_getter_type::null();
        }

        HANDLE _h;
        deleter_type _deleter;
    };
}
