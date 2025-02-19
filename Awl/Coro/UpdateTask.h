#pragma once

#include <coroutine>
#include <exception>
#include <utility>

#include "Awl/QuickLink.h"
#include "Awl/Coro/UpdatePromise.h"

namespace awl
{
    class UpdateTask
    {
    public:

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

        bool done() const
        {
            return m_h.done();
        }

        friend auto operator co_await(const UpdateTask& update_task) noexcept
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

    private:

        void release();

        std::coroutine_handle<promise_type> m_h;

        friend class Controller;
    };
}
