#include "Awl/Coro/TaskPool.h"

#include <cassert>
#include <ranges>
#include <functional>
#include <stdexcept>

using namespace awl;

void TaskPool::spawn(Job&& task)
{
    // A couroutine has executed as a regular function.
    // (It did not co_await).
    if (!task.done())
    {
        _handlers.emplace_back(this, std::move(task));

        Handler& handler = _handlers.back();

        handler._task.subscribe(&handler);
    }
}

void TaskPool::cancel()
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

Job TaskPool::wait_all_task_experimental()
{
    while (!_handlers.empty())
    {
        Job task = std::move(_handlers.back()._task);

        // The vector contains an empty task and
        // and onFinished() should delete it correctly.
        // BUG: This task is not cancelled by TaskPool::cancel().
        co_await task;
    }
}

void TaskPool::Handler::onFinished()
{
    // The handler is going to be deleted, save its members.
    TaskPool* saved_this = pThis;

    std::vector<Handler>& handlers = saved_this->_handlers;

    const std::size_t index = this - handlers.data();

    assert(index < handlers.size());

    handlers.erase(handlers.begin() + index);

    // For wait_any().
    saved_this->notify(&TaskSink::onFinished);
}
