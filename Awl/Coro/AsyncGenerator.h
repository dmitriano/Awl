/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
// Copied from Lewis Baker cppcoro library on GitHub
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <exception>
#include <atomic>
#include <iterator>
#include <type_traits>
#include <coroutine>
#include <functional>
#include <cassert>
namespace awl
{
    template<typename T>
    class async_generator;

    namespace detail
    {
        template<typename T>
        class async_generator_iterator;
        class async_generator_yield_operation;
        class async_generator_advance_operation;

        class async_generator_promise_base
        {
        public:

            async_generator_promise_base() noexcept
                : _exception(nullptr)
            {
                // Other variables left intentionally uninitialised as they're
                // only referenced in certain states by which time they should
                // have been initialised.
            }

            async_generator_promise_base(const async_generator_promise_base& other) = delete;
            async_generator_promise_base& operator=(const async_generator_promise_base& other) = delete;

            std::suspend_always initial_suspend() const noexcept
            {
                return {};
            }

            async_generator_yield_operation final_suspend() noexcept;

            void unhandled_exception() noexcept
            {
                _exception = std::current_exception();
            }

            void return_void() noexcept
            {}

            /// Query if the generator has reached the end of the sequence.
            ///
            /// Only valid to call after resuming from an awaited advance operation.
            /// i.e. Either a begin() or iterator::operator++() operation.
            bool finished() const noexcept
            {
                return _currentValue == nullptr;
            }

            void rethrow_if_unhandled_exception()
            {
                if (_exception)
                {
                    std::rethrow_exception(std::move(_exception));
                }
            }

        protected:

            async_generator_yield_operation internal_yield_value() noexcept;

        private:

            friend class async_generator_yield_operation;
            friend class async_generator_advance_operation;

            std::exception_ptr _exception;

            std::coroutine_handle<> _consumerCoroutine;

        protected:

            void* _currentValue;
        };

        class async_generator_yield_operation final
        {
        public:

            async_generator_yield_operation(std::coroutine_handle<> consumer) noexcept
                : _consumer(consumer)
            {}

            bool await_ready() const noexcept
            {
                return false;
            }

            std::coroutine_handle<>
                await_suspend([[maybe_unused]] std::coroutine_handle<> producer) noexcept
            {
                return _consumer;
            }

            void await_resume() noexcept {}

        private:

            std::coroutine_handle<> _consumer;
        };

        inline async_generator_yield_operation async_generator_promise_base::final_suspend() noexcept
        {
            _currentValue = nullptr;
            return internal_yield_value();
        }

        inline async_generator_yield_operation async_generator_promise_base::internal_yield_value() noexcept
        {
            return async_generator_yield_operation{ _consumerCoroutine };
        }

        class async_generator_advance_operation
        {
        protected:

            async_generator_advance_operation(std::nullptr_t) noexcept
                : _promise(nullptr)
                , _producerCoroutine(nullptr)
            {}

            async_generator_advance_operation(
                async_generator_promise_base& promise,
                std::coroutine_handle<> producerCoroutine) noexcept
                : _promise(std::addressof(promise))
                , _producerCoroutine(producerCoroutine)
            {}

        public:

            bool await_ready() const noexcept { return false; }

            std::coroutine_handle<>
                await_suspend(std::coroutine_handle<> consumerCoroutine) noexcept
            {
                _promise->_consumerCoroutine = consumerCoroutine;
                return _producerCoroutine;
            }

        protected:

            async_generator_promise_base* _promise;
            std::coroutine_handle<> _producerCoroutine;
        };

        template<typename T>
        class async_generator_promise final : public async_generator_promise_base
        {
            using value_type = std::remove_reference_t<T>;

        public:

            async_generator_promise() noexcept = default;

            async_generator<T> get_return_object() noexcept;

            async_generator_yield_operation yield_value(value_type& value) noexcept
            {
                _currentValue = std::addressof(value);
                return internal_yield_value();
            }

            async_generator_yield_operation yield_value(value_type&& value) noexcept
            {
                return yield_value(value);
            }

            T& value() const noexcept
            {
                return *static_cast<T*>(_currentValue);
            }
        };

        template<typename T>
        class async_generator_promise<T&&> final : public async_generator_promise_base
        {
        public:

            async_generator_promise() noexcept = default;

            async_generator<T> get_return_object() noexcept;

            async_generator_yield_operation yield_value(T&& value) noexcept
            {
                _currentValue = std::addressof(value);
                return internal_yield_value();
            }

            T&& value() const noexcept
            {
                return std::move(*static_cast<T*>(_currentValue));
            }
        };

        template<typename T>
        class async_generator_increment_operation final : public async_generator_advance_operation
        {
        public:

            async_generator_increment_operation(async_generator_iterator<T>& iterator) noexcept
                : async_generator_advance_operation(iterator._coroutine.promise(), iterator._coroutine)
                , _iterator(iterator)
            {}

            async_generator_iterator<T>& await_resume();

        private:

            async_generator_iterator<T>& _iterator;
        };

        template<typename T>
        class async_generator_iterator final
        {
            using promise_type = async_generator_promise<T>;
            using handle_type = std::coroutine_handle<promise_type>;

        public:

            using iterator_category = std::input_iterator_tag;
            // Not sure what type should be used for difference_type as we don't
            // allow calculating difference between two iterators.
            using difference_type = std::ptrdiff_t;
            using value_type = std::remove_reference_t<T>;
            using reference = std::add_lvalue_reference_t<T>;
            using pointer = std::add_pointer_t<value_type>;

            async_generator_iterator(std::nullptr_t) noexcept
                : _coroutine(nullptr)
            {}

            async_generator_iterator(handle_type coroutine) noexcept
                : _coroutine(coroutine)
            {}

            async_generator_increment_operation<T> operator++() noexcept
            {
                return async_generator_increment_operation<T>{ *this };
            }

            reference operator*() const noexcept
            {
                return _coroutine.promise().value();
            }

            bool operator==(const async_generator_iterator& other) const noexcept
            {
                return _coroutine == other._coroutine;
            }

            bool operator!=(const async_generator_iterator& other) const noexcept
            {
                return !(*this == other);
            }

        private:

            friend class async_generator_increment_operation<T>;

            handle_type _coroutine;
        };

        template<typename T>
        async_generator_iterator<T>& async_generator_increment_operation<T>::await_resume()
        {
            if (_promise->finished())
            {
                // Update iterator to end()
                _iterator = async_generator_iterator<T>{ nullptr };
                _promise->rethrow_if_unhandled_exception();
            }

            return _iterator;
        }

        template<typename T>
        class async_generator_begin_operation final : public async_generator_advance_operation
        {
            using promise_type = async_generator_promise<T>;
            using handle_type = std::coroutine_handle<promise_type>;

        public:

            async_generator_begin_operation(std::nullptr_t) noexcept
                : async_generator_advance_operation(nullptr)
            {}

            async_generator_begin_operation(handle_type producerCoroutine) noexcept
                : async_generator_advance_operation(producerCoroutine.promise(), producerCoroutine)
            {}

            bool await_ready() const noexcept
            {
                return _promise == nullptr || async_generator_advance_operation::await_ready();
            }

            async_generator_iterator<T> await_resume()
            {
                if (_promise == nullptr)
                {
                    // Called begin() on the empty generator.
                    return async_generator_iterator<T>{ nullptr };
                }
                else if (_promise->finished())
                {
                    // Completed without yielding any values.
                    _promise->rethrow_if_unhandled_exception();
                    return async_generator_iterator<T>{ nullptr };
                }

                return async_generator_iterator<T>{
                    handle_type::from_promise(*static_cast<promise_type*>(_promise))
                };
            }
        };
    }

    template<typename T>
    class [[nodiscard]] async_generator
    {
    public:

        using promise_type = detail::async_generator_promise<T>;
        using iterator = detail::async_generator_iterator<T>;

        async_generator() noexcept
            : _coroutine(nullptr)
        {}

        explicit async_generator(promise_type& promise) noexcept
            : _coroutine(std::coroutine_handle<promise_type>::from_promise(promise))
        {}

        async_generator(async_generator&& other) noexcept
            : _coroutine(other._coroutine)
        {
            other._coroutine = nullptr;
        }

        ~async_generator()
        {
            if (_coroutine)
            {
                _coroutine.destroy();
            }
        }

        async_generator& operator=(async_generator&& other) noexcept
        {
            async_generator temp(std::move(other));
            swap(temp);
            return *this;
        }

        async_generator(const async_generator&) = delete;
        async_generator& operator=(const async_generator&) = delete;

        auto begin() noexcept
        {
            if (!_coroutine)
            {
                return detail::async_generator_begin_operation<T>{ nullptr };
            }

            return detail::async_generator_begin_operation<T>{ _coroutine };
        }

        auto end() noexcept
        {
            return iterator{ nullptr };
        }

        void swap(async_generator& other) noexcept
        {
            std::swap(_coroutine, other._coroutine);
        }

    private:

        std::coroutine_handle<promise_type> _coroutine;
    };

    template<typename T>
    void swap(async_generator<T>& a, async_generator<T>& b) noexcept
    {
        a.swap(b);
    }

    namespace detail
    {
        template<typename T>
        async_generator<T> async_generator_promise<T>::get_return_object() noexcept
        {
            return async_generator<T>{ *this };
        }
    }
}
