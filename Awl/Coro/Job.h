#pragma once

#include <coroutine>
#include <exception>
#include <utility>
#include <cassert>

#include "Awl/Coro/JobPromise.h"
#include "Awl/QuickLink.h"

namespace awl
{
    class Job
    {
    public:

        // declare promise type
        using promise_type = JobPromise;

        Job() : m_h(nullptr) {}
        
        Job(std::coroutine_handle<promise_type> m_h) : m_h(m_h) {}

        Job(Job&& other) noexcept : m_h(std::exchange(other.m_h, nullptr)) {}

        Job& operator=(Job&& other) noexcept
        {
            free();

            m_h = std::exchange(other.m_h, nullptr);

            return *this;
        }

        bool operator == (const Job& other) const = default;

        ~Job()
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

        friend auto operator co_await(const Job& job) noexcept
        {
            if (!job.m_h)
            {
                //coroutine without promise awaited
                std::terminate();
            }

            if (job.m_h.promise().m_awaitingCoroutine)
            {
                //coroutine already awaited
                std::terminate();
            }

            struct task_awaitable
            {
                std::coroutine_handle<JobPromise> m_h;

                // check if this Job already has value computed
                bool await_ready()
                {
                    return m_h.done();
                }

                // h - is a handle to coroutine that calls co_await
                // store coroutine handle to be resumed after computing Job value
                void await_suspend(std::coroutine_handle<> h)
                {
                    m_h.promise().m_awaitingCoroutine = h;
                }

                // when ready return value to a consumer
                auto await_resume()
                {
                }
            };

            return task_awaitable{ job.m_h };
        }

        void subscribe(awl::Observer<TaskSink>* p_sink)
        {
            // The promise is owned at this point.
            assert(m_h != nullptr);

            Job::promise_type& promise = m_h.promise();

            promise.subscribe(p_sink);
        }

    private:

        //void release();

        std::coroutine_handle<promise_type> m_h;
    };
}
