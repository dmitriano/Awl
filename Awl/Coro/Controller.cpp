#pragma once

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
    assert(!task.done());

    UpdateTask::promise_type& promise = task.m_h.promise();

    m_handlers.emplace_back(this, std::move(task));

    promise.Subscribe(std::addressof(m_handlers.back()));
}

void Controller::cancel()
{
    for (Handler& handler : m_handlers)
    {
        handler.UnsubscribeSelf();
    }

    m_handlers.clear();
}

UpdateTask Controller::wait_all()
{
    while (!m_handlers.empty())
    {
        UpdateTask task = std::move(m_handlers.back().m_task);

        // The vector contains an empty task and
        // and OnFinished() should delete it correctly.
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
