#include "Awl/Coro/JobGroup.h"

#include <algorithm>
#include <cassert>
#include <ranges>
#include <functional>
#include <stdexcept>

namespace awl::coro
{
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

    std::size_t JobGroup::task_count() const
    {
        return std::ranges::count_if(
            _handlers,
            [](const Handler& handler)
            {
                return !handler.finished();
            });
    }

    bool JobGroup::empty() const
    {
        return task_count() == 0;
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

            // BUG: This task is not cancelled by JobGroup::cancel().
            if (task)
            {
                co_await task;
            }

            _handlers.pop_back();
        }
    }

    void JobGroup::Handler::onFinished()
    {
        _finished = true;

        // For wait_any().
        pThis->notify(&TaskSink::onFinished);
    }
}
