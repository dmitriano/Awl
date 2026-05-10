#pragma once

#include "Awl/Coro/JobPromise.h"
#include "Awl/QuickLink.h"

#include <coroutine>
#include <exception>
#include <utility>
#include <cassert>

namespace awl
{
    class Job
    {
    public:

        // declare promise type
        using promise_type = JobPromise;

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

        friend auto operator co_await(const Job& job) noexcept
        {
            if (!job._h)
            {
                //coroutine without promise awaited
                std::terminate();
            }

            if (job._h.promise()._awaitingCoroutine)
            {
                //coroutine already awaited
                std::terminate();
            }

            struct task_awaitable
            {
                std::coroutine_handle<JobPromise> _h;

                // check if this Job already has value computed
                bool await_ready()
                {
                    return _h.done();
                }

                // h - is a handle to coroutine that calls co_await
                // store coroutine handle to be resumed after computing Job value
                void await_suspend(std::coroutine_handle<> h)
                {
                    _h.promise()._awaitingCoroutine = h;
                }

                // when ready return value to a consumer
                auto await_resume()
                {}
            };

            return task_awaitable{ job._h };
        }

        void subscribe(awl::Observer<TaskSink>* p_sink)
        {
            // The promise is owned at this point.
            assert(_h != nullptr);

            Job::promise_type& promise = _h.promise();

            promise.subscribe(p_sink);
        }

    private:

        //void release();

        std::coroutine_handle<promise_type> _h;
    };
}
