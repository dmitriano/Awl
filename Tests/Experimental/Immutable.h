/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <stdexcept>
#include <cstddef>
#include <concepts>
#include <functional>
#include <utility>

namespace awl
{
    // When we need an object to be "const", but still movable, we wrap it into immutable.
    // This class is similar in its semantics to std::unique_ptr,
    // but it does not allocate dynamic memory.
    template <class T>
    class immutable
    {
    public:

        constexpr immutable(T val) noexcept : _val(std::move(val)) {}
        
        constexpr immutable(const immutable& other) : _val(other._val) {}

        constexpr immutable(immutable&& other) noexcept : _val(safeMove(other)) {}

        constexpr immutable& operator=(const immutable& other)
        {
            other.ensureNotMoved();

            _val = other._val;

            return *this;
        }

        constexpr immutable& operator=(immutable&& other) noexcept
        {
            _val = safeMove(other);

            return *this;
        }

        constexpr bool operator == (const immutable& other) const noexcept
        {
            ensureNotMoved();
            other.ensureNotMoved();

            return operator == (other._val);
        }

        constexpr bool operator == (const T& val) const noexcept
        {
            ensureNotMoved();

            return _val == val;
        }

        constexpr const T& operator*() const noexcept
        {
            ensureNotMoved();

            return _val;
        }

        constexpr const T* operator->() const noexcept
        {
            ensureNotMoved();

            return &_val;
        }

        template <class Func, class... Args>
            requires std::invocable<Func, T&, Args&&...>
        constexpr immutable with(Func&& func, Args&&... args) const &
        {
            ensureNotMoved();

            T val = _val;

            std::invoke(func, val, std::forward<Args>(args)...);

            return val;
        }

        template <class Func, class... Args>
            requires std::invocable<Func, T&, Args&&...>
        constexpr immutable with(Func&& func, Args&&... args) &&
        {
            T val = release();

            std::invoke(func, val, std::forward<Args>(args)...);

            return val;
        }

        template <class Field>
        constexpr immutable with(Field T::* p, Field&& field_val) const &
        {
            ensureNotMoved();

            T val = _val;

            val.*p = std::forward<Field>(field_val);

            return val;
        }

        template <class Field>
        constexpr immutable with(Field T::* p, Field&& field_val) &&
        {
            T val = release();

            val.*p = std::forward<Field>(field_val);

            return val;
        }

        constexpr T release()
        {
            return safeMove(*this);
        }

    private:

// This can be done via a template paramter like template<class T, Checker = FakeChecker<T>>
#ifdef AWL_DEBUG_IMMUTABLE
        
        constexpr void markAsMoved()
        {
            _owns = false;
        }

        constexpr void ensureNotMoved() const
        {
            if (!_owns)
            {
                // This will result in std::terminate, because operator->() is noexcept.
                throw std::runtime_error("An attempt to use an immutable object that was moved.");
            }
        }

        bool _owns = true;

#else
        
        constexpr void markAsMoved() {}

        constexpr void ensureNotMoved() const {}

#endif

        template <class Immutable>
        constexpr T&& safeMove(Immutable&& other) noexcept
        {
            other.ensureNotMoved();
            other.markAsMoved();

            return std::move(other._val);
        }

        T _val;
    };

    template <class T, class... Args>
    constexpr immutable<T> make_immutable(Args&&... args)
    {
        return T{ std::forward<Args>(args)... };
    }
}
