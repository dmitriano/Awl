#pragma once

#include <utility>
#include <coroutine>
#include <optional>

namespace awl
{
    template<typename T>
    struct ProcessTask;

    template<typename T>
    class ProcessPromise
    {
    public:

        // value to be computed
        // when ProcessTask is not completed (coroutine didn't co_return anything yet) value is empty
        std::optional<T> m_value;
        std::exception_ptr m_exception;

        void rethrow() const
        {
            if (m_exception)
            {
                std::rethrow_exception(m_exception);
            }
        }

        // corouine that awaiting this coroutine value
        // we need to store it in order to resume it later when value of this coroutine will be computed
        std::coroutine_handle<> m_awaitingCoroutine;

        // ProcessTask is async result of our coroutine
        // it is created before execution of the coroutine body
        // it can be either co_awaited inside another coroutine
        // or used via special interface for extracting values (is_ready and get)
        ProcessTask<T> get_return_object();

        // there are two kinds of coroutines:
        // 1. eager - that start its execution immediately
        // 2. lazy - that start its execution only after 'co_await'ing on them
        // here I used eager coroutine ProcessTask
        // eager: do not suspend before running coroutine body
        std::suspend_never initial_suspend() noexcept
        {
            return {};
        }

        // store value to be returned to awaiting coroutine or accessed through 'get' function
        void return_value(T val) noexcept
        {
            rethrow();
            
            m_value = std::move(val);
        }

        void unhandled_exception() noexcept
        {
            // alternatively we can store current exeption in std::exception_ptr to rethrow it later
            m_exception = std::current_exception();
        }

        // when final suspend is executed 'value' is already set
        // we need to suspend this coroutine in order to use value in other coroutine or through 'get' function
        // otherwise promise object would be destroyed (together with stored value) and one couldn't access ProcessTask result
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

                std::coroutine_handle<> await_suspend(std::coroutine_handle<ProcessPromise> h) noexcept
                {
                    ProcessPromise& promise = h.promise();

                    // resume awaiting coroutine or if there is no coroutine to resume return special coroutine that do
                    // nothing
                    return promise.m_awaitingCoroutine ? promise.m_awaitingCoroutine : std::noop_coroutine();
                }
                
                void await_resume() noexcept {}
            };
            
            return transfer_awaitable{};
        }
    };

    template<>
    class ProcessPromise<void>
    {
    public:

        std::exception_ptr m_exception;

        void rethrow() const
        {
            if (m_exception)
            {
                std::rethrow_exception(m_exception);
            }
        }

        // corouine that awaiting this coroutine value
        // we need to store it in order to resume it later when value of this coroutine will be computed
        std::coroutine_handle<> m_awaitingCoroutine;

        // ProcessTask is async result of our coroutine
        // it is created before execution of the coroutine body
        // it can be either co_awaited inside another coroutine
        // or used via special interface for extracting values (is_ready and get)
        ProcessTask<void> get_return_object();

        // there are two kinds of coroutines:
        // 1. eager - that start its execution immediately
        // 2. lazy - that start its execution only after 'co_await'ing on them
        // here I used eager coroutine ProcessTask
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
            m_exception = std::current_exception();
        }

        // when final suspend is executed 'value' is already set
        // we need to suspend this coroutine in order to use value in other coroutine or through 'get' function
        // otherwise promise object would be destroyed (together with stored value) and one couldn't access ProcessTask result
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

                std::coroutine_handle<> await_suspend(std::coroutine_handle<ProcessPromise> h) noexcept
                {
                    ProcessPromise& promise = h.promise();

                    // resume awaiting coroutine or if there is no coroutine to resume return special coroutine that do
                    // nothing
                    return promise.m_awaitingCoroutine ? promise.m_awaitingCoroutine : std::noop_coroutine();
                }

                void await_resume() noexcept {}
            };

            return transfer_awaitable{};
        }
    };

    template<typename T>
    struct ProcessTask
    {
        // declare promise type
        using promise_type = ProcessPromise<T>;

        ProcessTask() : m_h(nullptr) {}
        
        ProcessTask(std::coroutine_handle<promise_type> handle) noexcept : m_h(handle) {}

        ProcessTask(ProcessTask&& other) noexcept : m_h(std::exchange(other.m_h, nullptr)) {}

        ProcessTask& operator=(ProcessTask&& other) noexcept
        {
            free();

            m_h = std::exchange(other.m_h, nullptr);

            return *this;
        }

        ~ProcessTask()
        {
            free();
        }

        // interface for extracting value without awaiting on it

        bool is_ready() const
        {
            check_handle();

            m_h.promise().rethrow();

            return m_h.promise().m_value.has_value();
        }

        T get()
        {
            check_handle();
            
            m_h.promise().rethrow();

            return std::move(*m_h.promise().m_value);
        }

        void check_handle() const
        {
            if (!m_h)
            {
                //Uninitialized ProcessTask without promise
                std::terminate();
            }
        }

        void free()
        {
            if (m_h)
            {
                m_h.destroy();

                m_h = nullptr;
            }
        }

        std::coroutine_handle<promise_type> m_h;
    };

    template<typename T>
    ProcessTask<T> ProcessPromise<T>::get_return_object()
    {
        return { std::coroutine_handle<ProcessPromise>::from_promise(*this) };
    }

    inline ProcessTask<void> ProcessPromise<void>::get_return_object()
    {
        return { std::coroutine_handle<ProcessPromise>::from_promise(*this) };
    }

    // also we can await other ProcessTask<T>
    template<typename T>
    auto operator co_await(const ProcessTask<T>& task) noexcept
    {
        if (!task.m_h)
        {
            //coroutine without promise awaited
            std::terminate();
        }

        if (task.m_h.promise().m_awaitingCoroutine)
        {
            //coroutine already awaited
            std::terminate();
        }

        struct task_awaitable
        {
            std::coroutine_handle<ProcessPromise<T>> m_h;

            // check if this ProcessTask already has value computed
            bool await_ready()
            {
                return m_h.done();
                //return handle.promise().value.has_value();
            }

            // h - is a handle to coroutine that calls co_await
            // store coroutine handle to be resumed after computing ProcessTask value
            void await_suspend(std::coroutine_handle<> h)
            {
                m_h.promise().m_awaitingCoroutine = h;
            }

            // when ready return value to a consumer
            auto await_resume()
            {
                m_h.promise().rethrow();

                if constexpr (!std::is_same_v<T, void>)
                {
                    return std::move(*(m_h.promise().m_value));
                }
            }
        };

        return task_awaitable{ task.m_h };
    }
}
