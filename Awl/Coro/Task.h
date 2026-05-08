#pragma once

#include <utility>
#include <coroutine>
#include <optional>
#include <exception>

namespace awl
{
    template<typename T>
    struct Task;

    template<typename T>
    class TaskPromise
    {
    public:

        // value to be computed
        // when Task is not completed (coroutine didn't co_return anything yet) value is empty
        std::optional<T> _value;
        std::exception_ptr _exception;

        void rethrow() const
        {
            if (_exception)
            {
                std::rethrow_exception(_exception);
            }
        }

        // corouine that awaiting this coroutine value
        // we need to store it in order to resume it later when value of this coroutine will be computed
        std::coroutine_handle<> _awaitingCoroutine;

        // Task is async result of our coroutine
        // it is created before execution of the coroutine body
        // it can be either co_awaited inside another coroutine
        // or used via special interface for extracting values (is_ready and get)
        Task<T> get_return_object();

        // there are two kinds of coroutines:
        // 1. eager - that start its execution immediately
        // 2. lazy - that start its execution only after 'co_await'ing on them
        // here I used eager coroutine Task
        // eager: do not suspend before running coroutine body
        std::suspend_never initial_suspend() noexcept
        {
            return {};
        }

        // store value to be returned to awaiting coroutine or accessed through 'get' function
        void return_value(T val) noexcept
        {
            rethrow();
            
            _value = std::move(val);
        }

        void unhandled_exception() noexcept
        {
            // alternatively we can store current exeption in std::exception_ptr to rethrow it later
            _exception = std::current_exception();
        }

        // when final suspend is executed 'value' is already set
        // we need to suspend this coroutine in order to use value in other coroutine or through 'get' function
        // otherwise promise object would be destroyed (together with stored value) and one couldn't access Task result
        // value
        auto final_suspend() noexcept
        {
            // if there is a coroutine that is awaiting on this coroutine resume it
            struct transfer_awaitable
            {
                // always stop at final suspend
                bool await_ready() noexcept
                {
                    return false;
                }

                std::coroutine_handle<> await_suspend(std::coroutine_handle<TaskPromise> h) noexcept
                {
                    TaskPromise& promise = h.promise();

                    // resume awaiting coroutine or if there is no coroutine to resume return special coroutine that do
                    // nothing
                    return promise._awaitingCoroutine ? promise._awaitingCoroutine : std::noop_coroutine();
                }
                
                void await_resume() noexcept {}
            };
            
            return transfer_awaitable{};
        }
    };

    template<>
    class TaskPromise<void>
    {
    public:

        std::exception_ptr _exception;

        void rethrow() const
        {
            if (_exception)
            {
                std::rethrow_exception(_exception);
            }
        }

        // corouine that awaiting this coroutine value
        // we need to store it in order to resume it later when value of this coroutine will be computed
        std::coroutine_handle<> _awaitingCoroutine;

        // Task is async result of our coroutine
        // it is created before execution of the coroutine body
        // it can be either co_awaited inside another coroutine
        // or used via special interface for extracting values (is_ready and get)
        Task<void> get_return_object();

        // there are two kinds of coroutines:
        // 1. eager - that start its execution immediately
        // 2. lazy - that start its execution only after 'co_await'ing on them
        // here I used eager coroutine Task
        // eager: do not suspend before running coroutine body
        std::suspend_never initial_suspend()
        {
            return {};
        }

        // store value to be returned to awaiting coroutine or accessed through 'get' function
        void return_void()
        {
            rethrow();
        }

        void unhandled_exception()
        {
            // alternatively we can store current exeption in std::exception_ptr to rethrow it later
            _exception = std::current_exception();
        }

        // when final suspend is executed 'value' is already set
        // we need to suspend this coroutine in order to use value in other coroutine or through 'get' function
        // otherwise promise object would be destroyed (together with stored value) and one couldn't access Task result
        // value
        auto final_suspend() noexcept
        {
            // if there is a coroutine that is awaiting on this coroutine resume it
            struct transfer_awaitable
            {
                // always stop at final suspend
                bool await_ready() noexcept
                {
                    return false;
                }

                std::coroutine_handle<> await_suspend(std::coroutine_handle<TaskPromise> h) noexcept
                {
                    TaskPromise& promise = h.promise();

                    // resume awaiting coroutine or if there is no coroutine to resume return special coroutine that do
                    // nothing
                    return promise._awaitingCoroutine ? promise._awaitingCoroutine : std::noop_coroutine();
                }

                void await_resume() noexcept {}
            };

            return transfer_awaitable{};
        }
    };

    template<typename T>
    struct Task
    {
        // declare promise type
        using promise_type = TaskPromise<T>;

        Task() : _h(nullptr) {}
        
        Task(std::coroutine_handle<promise_type> handle) noexcept : _h(handle) {}

        Task(Task&& other) noexcept : _h(std::exchange(other._h, nullptr)) {}

        Task& operator=(Task&& other) noexcept
        {
            free();

            _h = std::exchange(other._h, nullptr);

            return *this;
        }

        ~Task()
        {
            free();
        }

        // interface for extracting value without awaiting on it

        bool is_ready() const
        {
            check_handle();

            _h.promise().rethrow();

            return _h.promise()._value.has_value();
        }

        T get()
        {
            check_handle();
            
            _h.promise().rethrow();

            return std::move(*_h.promise()._value);
        }

        void check_handle() const
        {
            if (!_h)
            {
                //Uninitialized Task without promise
                std::terminate();
            }
        }

        void free()
        {
            if (_h)
            {
                _h.destroy();

                _h = nullptr;
            }
        }

        std::coroutine_handle<promise_type> _h;
    };

    template<typename T>
    Task<T> TaskPromise<T>::get_return_object()
    {
        return { std::coroutine_handle<TaskPromise>::from_promise(*this) };
    }

    inline Task<void> TaskPromise<void>::get_return_object()
    {
        return { std::coroutine_handle<TaskPromise>::from_promise(*this) };
    }

    // also we can await other Task<T>
    template<typename T>
    auto operator co_await(const Task<T>& task) noexcept
    {
        if (!task._h)
        {
            //coroutine without promise awaited
            std::terminate();
        }

        if (task._h.promise()._awaitingCoroutine)
        {
            //coroutine already awaited
            std::terminate();
        }

        struct task_awaitable
        {
            std::coroutine_handle<TaskPromise<T>> _h;

            // check if this Task already has value computed
            bool await_ready()
            {
                return _h.done();
                //return handle.promise().value.has_value();
            }

            // h - is a handle to coroutine that calls co_await
            // store coroutine handle to be resumed after computing Task value
            void await_suspend(std::coroutine_handle<> h)
            {
                _h.promise()._awaitingCoroutine = h;
            }

            // when ready return value to a consumer
            auto await_resume()
            {
                _h.promise().rethrow();

                if constexpr (!std::is_same_v<T, void>)
                {
                    return std::move(*(_h.promise()._value));
                }
            }
        };

        return task_awaitable{ task._h };
    }
}
