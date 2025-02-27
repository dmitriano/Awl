#include "Awl/Coro/Controller.h"

#include <cassert>
#include <ranges>
#include <functional>
#include <stdexcept>

using namespace awl;

void Controller::register_task(UpdateTask&& task)
{
    // The promise is owned at this point.
    assert(task.m_h != nullptr);
    
    // A couroutine has executed as a regular function.
    // (It did not co_await).
    if (!task.done())
    {
        UpdateTask::promise_type& promise = task.m_h.promise();

        m_handlers.emplace_back(this, std::move(task));

        promise.Subscribe(std::addressof(m_handlers.back()));
    }
}

void Controller::cancel()
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

UpdateTask Controller::wait_all_task_experimental()
{
    while (!m_handlers.empty())
    {
        UpdateTask task = std::move(m_handlers.back().m_task);

        // The vector contains an empty task and
        // and OnFinished() should delete it correctly.
        // BUG: This task is not cancelled by Controller::cancel().
        co_await task;
    }
}

void Controller::Handler::OnFinished()
{
    const std::size_t index = this - pThis->m_handlers.data();

    assert(index < pThis->m_handlers.size());

    pThis->m_handlers.erase(pThis->m_handlers.begin() + index);

    // For wait_any().
    pThis->Notify(&TaskSink::OnFinished);
}
