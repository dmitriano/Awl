/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
// Copied from Lewis Baker cppcoro library on GitHub
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <coroutine>
#include <type_traits>
#include <utility>
#include <exception>
#include <iterator>
#include <functional>

namespace awl::coro
{
    template<typename T>
    class generator;

    namespace detail
    {
        template<typename T>
        class generator_promise
        {
        public:

            using value_type = std::remove_reference_t<T>;
            using reference_type = std::conditional_t<std::is_reference_v<T>, T, T&>;
            using pointer_type = value_type*;

            generator_promise() = default;

            generator<T> get_return_object() noexcept;

            constexpr std::suspend_always initial_suspend() const noexcept { return {}; }
            constexpr std::suspend_always final_suspend() const noexcept { return {}; }

            template<
                typename U = T,
                std::enable_if_t<!std::is_rvalue_reference<U>::value, int> = 0>
            std::suspend_always yield_value(std::remove_reference_t<T>& value) noexcept
            {
                _value = std::addressof(value);
                return {};
            }

            std::suspend_always yield_value(std::remove_reference_t<T>&& value) noexcept
            {
                _value = std::addressof(value);
                return {};
            }

            void unhandled_exception()
            {
                _exception = std::current_exception();
            }

            void return_void()
            {}

            reference_type value() const noexcept
            {
                return static_cast<reference_type>(*_value);
            }

            // Don't allow any use of 'co_await' inside the generator coroutine.
            template<typename U>
            std::suspend_never await_transform(U&& value) = delete;

            void rethrow_if_exception()
            {
                if (_exception)
                {
                    std::rethrow_exception(_exception);
                }
            }

        private:

            pointer_type _value;
            std::exception_ptr _exception;
        };

        struct generator_sentinel {};

        template<typename T>
        class generator_iterator
        {
            using coroutine_handle = std::coroutine_handle<generator_promise<T>>;

        public:

            using iterator_category = std::input_iterator_tag;
            // What type should we use for counting elements of a potentially infinite sequence?
            using difference_type = std::ptrdiff_t;
            using value_type = typename generator_promise<T>::value_type;
            using reference = typename generator_promise<T>::reference_type;
            using pointer = typename generator_promise<T>::pointer_type;

            // Iterator needs to be default-constructible to satisfy the Range concept.
            generator_iterator() noexcept
                : _coroutine(nullptr)
            {}

            explicit generator_iterator(coroutine_handle coroutine) noexcept
                : _coroutine(coroutine)
            {}

            friend bool operator==(const generator_iterator& it, generator_sentinel) noexcept
            {
                return !it._coroutine || it._coroutine.done();
            }

            friend bool operator!=(const generator_iterator& it, generator_sentinel s) noexcept
            {
                return !(it == s);
            }

            friend bool operator==(generator_sentinel s, const generator_iterator& it) noexcept
            {
                return (it == s);
            }

            friend bool operator!=(generator_sentinel s, const generator_iterator& it) noexcept
            {
                return it != s;
            }

            generator_iterator& operator++()
            {
                _coroutine.resume();
                if (_coroutine.done())
                {
                    _coroutine.promise().rethrow_if_exception();
                }

                return *this;
            }

            // Need to provide post-increment operator to implement the 'Range' concept.
            void operator++(int)
            {
                (void)operator++();
            }

            reference operator*() const noexcept
            {
                return _coroutine.promise().value();
            }

            pointer operator->() const noexcept
            {
                return std::addressof(operator*());
            }

        private:

            coroutine_handle _coroutine;
        };
    }

    template<typename T>
    class [[nodiscard]] generator
    {
    public:

        using promise_type = detail::generator_promise<T>;
        using iterator = detail::generator_iterator<T>;

        generator() noexcept
            : _coroutine(nullptr)
        {}

        generator(generator&& other) noexcept
            : _coroutine(other._coroutine)
        {
            other._coroutine = nullptr;
        }

        generator(const generator& other) = delete;

        ~generator()
        {
            if (_coroutine)
            {
                _coroutine.destroy();
            }
        }

        generator& operator=(generator other) noexcept
        {
            swap(other);
            return *this;
        }

        iterator begin()
        {
            if (_coroutine)
            {
                _coroutine.resume();
                if (_coroutine.done())
                {
                    _coroutine.promise().rethrow_if_exception();
                }
            }

            return iterator{ _coroutine };
        }

        detail::generator_sentinel end() noexcept
        {
            return detail::generator_sentinel{};
        }

        void swap(generator& other) noexcept
        {
            std::swap(_coroutine, other._coroutine);
        }

    private:

        friend class detail::generator_promise<T>;

        explicit generator(std::coroutine_handle<promise_type> coroutine) noexcept
            : _coroutine(coroutine)
        {}

        std::coroutine_handle<promise_type> _coroutine;
    };

    template<typename T>
    void swap(generator<T>& a, generator<T>& b)
    {
        a.swap(b);
    }

    namespace detail
    {
        template<typename T>
        generator<T> generator_promise<T>::get_return_object() noexcept
        {
            using coroutine_handle = std::coroutine_handle<generator_promise<T>>;
            return generator<T>{ coroutine_handle::from_promise(*this) };
        }
    }

    template<typename FUNC, typename T>
    generator<std::invoke_result_t<FUNC&, typename generator<T>::iterator::reference>> fmap(FUNC func, generator<T> source)
    {
        for (auto&& value : source)
        {
            co_yield std::invoke(func, static_cast<decltype(value)>(value));
        }
    }
}
