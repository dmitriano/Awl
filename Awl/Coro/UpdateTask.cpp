#pragma once

#include "Awl/Coro/UpdateTask.h"

namespace awl
{
    auto operator co_await(const UpdateTask& update_task) noexcept
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

using namespace awl;

UpdateTask UpdatePromise::get_return_object()
{
    return { std::coroutine_handle<UpdatePromise>::from_promise(*this) };
}

void UpdateTask::release()
{
    if (m_h)
    {
        m_h.promise().m_owned = false;

        m_h = nullptr;
    }
}
