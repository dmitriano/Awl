#pragma once

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
        std::optional<T> value;
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
        std::suspend_never initial_suspend()
        {
            return {};
        }

        // store value to be returned to awaiting coroutine or accessed through 'get' function
        void return_value(T val)
        {
            rethrow();
            
            value = std::move(val);
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

        ProcessTask(std::coroutine_handle<promise_type> handle) noexcept : m_handle(handle) {}

        ProcessTask(ProcessTask&& other) noexcept : m_handle(std::exchange(other.m_handle, nullptr)) {}

        ProcessTask& operator=(ProcessTask&& other) noexcept
        {
            free();

            m_handle = std::exchange(other.m_handle, nullptr);

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

            m_handle.promise().rethrow();

            return m_handle.promise().value.has_value();

            return false;
        }

        T get()
        {
            check_handle();
            
            m_handle.promise().rethrow();

            return std::move(*m_handle.promise().value);
        }

        void check_handle() const
        {
            if (!m_handle)
            {
                //Uninitialized ProcessTask without promise
                std::terminate();
            }
        }

        void free()
        {
            if (m_handle)
            {
                m_handle.destroy();

                m_handle = nullptr;
            }
        }

        std::coroutine_handle<promise_type> m_handle;
    };

    template<typename T>
    ProcessTask<T> ProcessPromise<T>::get_return_object()
    {
        return { std::coroutine_handle<ProcessPromise>::from_promise(*this) };
    }

    // also we can await other ProcessTask<T>
    template<typename U>
    auto operator co_await(const ProcessTask<U>& http_task) noexcept
    {
        if (!http_task.m_handle)
        {
            //coroutine without promise awaited
            std::terminate();
        }

        if (http_task.m_handle.promise().m_awaitingCoroutine)
        {
            //coroutine already awaited
            std::terminate();
        }

        struct task_awaitable
        {
            std::coroutine_handle<ProcessPromise<U>> handle;

            // check if this ProcessTask already has value computed
            bool await_ready()
            {
                return handle.done();
                //return handle.promise().value.has_value();
            }

            // h - is a handle to coroutine that calls co_await
            // store coroutine handle to be resumed after computing ProcessTask value
            void await_suspend(std::coroutine_handle<> h)
            {
                handle.promise().m_awaitingCoroutine = h;
            }

            // when ready return value to a consumer
            auto await_resume()
            {
                handle.promise().rethrow();

                return std::move(*(handle.promise().value));
            }
        };

        return task_awaitable{ http_task.m_handle };
    }
}
