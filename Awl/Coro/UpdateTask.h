#pragma once

#include <coroutine>
#include <exception>
#include <utility>

namespace awl
{
    struct UpdatePromise;

    struct UpdateTask
    {
        // declare promise type
        using promise_type = UpdatePromise;

        UpdateTask() : m_h(nullptr) {}
        
        UpdateTask(std::coroutine_handle<promise_type> m_h) : m_h(m_h) {}

        UpdateTask(UpdateTask&& other) noexcept : m_h(std::exchange(other.m_h, nullptr)) {}

        UpdateTask& operator=(UpdateTask&& other) noexcept
        {
            free();

            m_h = std::exchange(other.m_h, nullptr);

            return *this;
        }

        ~UpdateTask()
        {
            free();
        }

        void release();

        void free()
        {
            if (m_h)
            {
                m_h.destroy();

                m_h = nullptr;
            }
        }

        operator bool() const
        {
            return m_h != nullptr;
        }

        std::coroutine_handle<promise_type> m_h;
    };

    struct UpdatePromise
    {
        // corouine that awaiting this coroutine value
        // we need to store it in order to resume it later when value of this coroutine will be computed
        std::coroutine_handle<> m_awaitingCoroutine;

        bool m_owned = true;

        // UpdateTask is async result of our coroutine
        // it is created before execution of the coroutine body
        // it can be either co_awaited inside another coroutine
        // or used via special interface for extracting values (is_ready and get)
        UpdateTask get_return_object();

        // there are two kinds of coroutines:
        // 1. eager - that start its execution immediately
        // 2. lazy - that start its execution only after 'co_await'ing on them
        // here I used eager coroutine UpdateTask
        // eager: do not suspend before running coroutine body
        std::suspend_never initial_suspend()
        {
            return {};
        }

        void unhandled_exception()
        {
            // alternatively we can store current exeption in std::exception_ptr to rethrow it later
            std::terminate();
        }

        // when final suspend is executed 'value' is already set
        // we need to suspend this coroutine in order to use value in other coroutine or through 'get' function
        // otherwise promise object would be destroyed (together with stored value) and one couldn't access UpdateTask result
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

                // resume awaiting coroutine or if there is no coroutine to resume return special coroutine that do
                // nothing
                //std::coroutine_handle<> await_suspend(std::coroutine_handle<UpdatePromise> h) noexcept
                //{
                //    UpdatePromise& promise = h.promise();

                //    return promise.m_awaitingCoroutine ? promise.m_awaitingCoroutine : std::noop_coroutine();
                //}
                void await_suspend(std::coroutine_handle<UpdatePromise> h) noexcept
                {
                    UpdatePromise& promise = h.promise();

                    auto coro = promise.m_awaitingCoroutine;

                    if (!promise.m_owned)
                    {
                        h.destroy();
                    }

                    if (coro)
                    {
                        coro.resume();
                    }
                }

                void await_resume() noexcept {}
            };
            
            return transfer_awaitable{};
        }

        void return_void() {}
    };

    inline UpdateTask UpdatePromise::get_return_object()
    {
        return { std::coroutine_handle<UpdatePromise>::from_promise(*this) };
    }

    inline void UpdateTask::release()
    {
        if (m_h)
        {
            m_h.promise().m_owned = false;

            m_h = nullptr;
        }
    }

    inline auto operator co_await(const UpdateTask& update_task) noexcept
    {
        if (!update_task.m_h)
        {
            //coroutine without promise awaited
            std::terminate();
        }

        if (update_task.m_h.promise().m_awaitingCoroutine)
        {
            //coroutine already awaited
            std::terminate();
        }

        struct task_awaitable
        {
            std::coroutine_handle<UpdatePromise> m_h;

            // check if this UpdateTask already has value computed
            bool await_ready()
            {
                return m_h.done();
            }

            // h - is a handle to coroutine that calls co_await
            // store coroutine handle to be resumed after computing UpdateTask value
            void await_suspend(std::coroutine_handle<> h)
            {
                m_h.promise().m_awaitingCoroutine = h;
            }

            // when ready return value to a consumer
            auto await_resume()
            {
            }
        };

        return task_awaitable{ update_task.m_h };
    }
}
