#pragma once

#include "Awl/Coro/JobPromise.h"
#include "Awl/QuickLink.h"

#include <cassert>
#include <coroutine>
#include <exception>
#include <functional>
#include <memory>
#include <optional>
#include <utility>

namespace awl::coro
{
    class Job
    {
    public:

        // declare promise type
        using promise_type = detail::JobPromise;

        Job() : _h(nullptr) {}

        Job(std::coroutine_handle<promise_type> _h) : _h(_h) {}

        Job(Job&& other) noexcept : _h(std::exchange(other._h, nullptr)) {}

        Job& operator=(Job&& other) noexcept
        {
            free();

            _h = std::exchange(other._h, nullptr);

            return *this;
        }

        bool operator == (const Job& other) const = default;

        ~Job()
        {
            free();
        }

        void free()
        {
            if (_h)
            {
                _h.destroy();

                _h = nullptr;
            }
        }

        operator bool() const
        {
            return _h != nullptr;
        }

        bool done() const
        {
            return _h.done();
        }

        auto await(std::shared_ptr<IDispatcher> awaiting_dispatcher) & noexcept
        {
            if (!_h)
            {
                //coroutine without promise awaited
                std::terminate();
            }

            struct job_awaitable
            {
                std::coroutine_handle<detail::JobPromise> _h;
                std::shared_ptr<IDispatcher> _awaitingDispatcher;

                // check if this Job already has value computed
                bool await_ready()
                {
                    return _h.done();
                }

                // h - is a handle to coroutine that calls co_await
                // store coroutine handle to be resumed after computing Job value
                bool await_suspend(std::coroutine_handle<> h)
                {
                    detail::JobPromise& promise = _h.promise();

                    if (promise._awaitingCoroutine)
                    {
                        //coroutine already awaited
                        std::terminate();
                    }

                    promise._awaitingCoroutine = h;
                    promise._awaitingDispatcher = _awaitingDispatcher;
                    promise._resumeAwaitingOnFinalSuspend = false;

                    detail::start(_h);

                    if (_h.done())
                    {
                        promise._awaitingCoroutine = nullptr;
                        promise._awaitingDispatcher = nullptr;
                        promise._resumeAwaitingOnFinalSuspend = true;

                        return false;
                    }

                    promise._resumeAwaitingOnFinalSuspend = true;

                    return true;
                }

                // when ready return value to a consumer
                auto await_resume()
                {}
            };

            return job_awaitable{ _h, std::move(awaiting_dispatcher) };
        }

        auto await(std::shared_ptr<IDispatcher> awaiting_dispatcher) && noexcept
        {
            if (!_h)
            {
                //coroutine without promise awaited
                std::terminate();
            }

            struct owning_job_awaitable
            {
                std::optional<Job> _job;
                std::coroutine_handle<detail::JobPromise> _h;
                std::shared_ptr<IDispatcher> _awaitingDispatcher;

                bool await_ready()
                {
                    return _h.done();
                }

                bool await_suspend(std::coroutine_handle<> h)
                {
                    detail::JobPromise& promise = _h.promise();

                    if (promise._awaitingCoroutine)
                    {
                        std::terminate();
                    }

                    promise._awaitingCoroutine = h;
                    promise._awaitingDispatcher = _awaitingDispatcher;
                    promise._resumeAwaitingOnFinalSuspend = false;

                    detail::start(_h);

                    if (_h.done())
                    {
                        promise._awaitingCoroutine = nullptr;
                        promise._awaitingDispatcher = nullptr;
                        promise._resumeAwaitingOnFinalSuspend = true;

                        return false;
                    }

                    promise._resumeAwaitingOnFinalSuspend = true;

                    return true;
                }

                auto await_resume()
                {}
            };

            Job job = std::move(*this);
            std::coroutine_handle<detail::JobPromise> handle = job._h;

            return owning_job_awaitable{ std::move(job), handle, std::move(awaiting_dispatcher) };
        }

        void subscribe(awl::Observer<TaskSink>* p_sink)
        {
            // The promise is owned at this point.
            assert(_h != nullptr);

            Job::promise_type& promise = _h.promise();

            promise.subscribe(p_sink);
        }

    private:

        friend Job spawn(Job job, std::shared_ptr<IDispatcher> dispatcher);
        friend detail::JobPromise;

        //void release();

        std::coroutine_handle<promise_type> _h;
    };

    inline Job spawn(Job job, std::shared_ptr<IDispatcher> dispatcher = nullptr)
    {
        if (!job._h)
        {
            std::terminate();
        }

        job._h.promise().setDispatcher(std::move(dispatcher));
        detail::start(job._h);

        return job;
    }

    template<class Func>
    auto spawn(Func func, std::shared_ptr<IDispatcher> dispatcher = nullptr)
        requires std::invocable<Func&>
    {
        return spawn(std::invoke(func), std::move(dispatcher));
    }

    inline auto detail::JobPromise::await_transform(Job& job) noexcept
    {
        job._h.promise().setDispatcherIfEmpty(_dispatcher);

        return job.await(_dispatcher);
    }

    inline auto detail::JobPromise::await_transform(Job&& job) noexcept
    {
        job._h.promise().setDispatcherIfEmpty(_dispatcher);

        return std::move(job).await(_dispatcher);
    }
}
