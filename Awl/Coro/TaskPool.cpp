#include "Awl/Coro/TaskPool.h"

#include <cassert>
#include <ranges>
#include <functional>
#include <stdexcept>

using namespace awl;

void TaskPool::add_task(UpdateTask&& task)
{
    // A couroutine has executed as a regular function.
    // (It did not co_await).
    if (!task.done())
    {
        m_handlers.emplace_back(this, std::move(task));

        Handler& handler = m_handlers.back();

        handler.m_task.subscribe(&handler);
    }
}

void TaskPool::cancel()
{
    // Do not notify awaiters if nothing changed.
    if (!m_handlers.empty())
    {
        for (Handler& handler : m_handlers)
        {
            handler.UnsubscribeSelf();
        }

        m_handlers.clear();

        Notify(&TaskSink::OnFinished);
    }
}

UpdateTask TaskPool::wait_all_task_experimental()
{
    while (!m_handlers.empty())
    {
        UpdateTask task = std::move(m_handlers.back().m_task);

        // The vector contains an empty task and
        // and OnFinished() should delete it correctly.
        // BUG: This task is not cancelled by TaskPool::cancel().
        co_await task;
    }
}

void TaskPool::Handler::OnFinished()
{
    const std::size_t index = this - pThis->m_handlers.data();

    assert(index < pThis->m_handlers.size());

    pThis->m_handlers.erase(pThis->m_handlers.begin() + index);

    // For wait_any().
    pThis->Notify(&TaskSink::OnFinished);
}
