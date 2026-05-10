#include "Awl/Coro/JobGroup.h"

#include <cassert>
#include <ranges>
#include <functional>
#include <stdexcept>

using namespace awl;

JobGroup::JobGroup(JobGroup&& other) noexcept :
    Observable<TaskSink>(std::move(other)),
    _handlers(std::move(other._handlers))
{
    updateHandlersOwner();
}

JobGroup& JobGroup::operator = (JobGroup&& other) noexcept
{
    Observable<TaskSink>::operator = (std::move(other));
    _handlers = std::move(other._handlers);
    updateHandlersOwner();

    return *this;
}

void JobGroup::spawn(Job&& task)
{
    // A coroutine has executed as a regular function.
    // (It did not co_await).
    if (!task.done())
    {
        _handlers.emplace_back(this, std::move(task));

        Handler& handler = _handlers.back();

        handler._task.subscribe(&handler);
    }
}

void JobGroup::cancel()
{
    // Do not notify awaiters if nothing changed.
    if (!_handlers.empty())
    {
        for (Handler& handler : _handlers)
        {
            handler.unsubscribeSelf();
        }

        _handlers.clear();

        notify(&TaskSink::onFinished);
    }
}

void JobGroup::updateHandlersOwner()
{
    for (Handler& handler : _handlers)
    {
        handler.pThis = this;
    }
}

Job JobGroup::wait_all_jobs_experimental()
{
    while (!_handlers.empty())
    {
        Job task = std::move(_handlers.back()._task);

        // The vector contains an empty task and
        // onFinished() should delete it correctly.
        // BUG: This task is not cancelled by JobGroup::cancel().
        co_await task;
    }
}

void JobGroup::Handler::onFinished()
{
    // The handler is going to be deleted, save its members.
    JobGroup* saved_this = pThis;

    std::vector<Handler>& handlers = saved_this->_handlers;

    const std::size_t index = this - handlers.data();

    assert(index < handlers.size());

    handlers.erase(handlers.begin() + index);

    // For wait_any().
    saved_this->notify(&TaskSink::onFinished);
}
