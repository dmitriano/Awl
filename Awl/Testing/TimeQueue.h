/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <chrono>
#include <queue>
#include <vector>
#include <thread>
#include <coroutine>
#include <limits>

#include "Awl/KeyCompare.h"

namespace awl::testing
{
    class TimeQueue
    {
    public:

        void push(std::coroutine_handle<> handle, std::chrono::nanoseconds timeout)
        {
            m_tasks.push(Task{ std::chrono::steady_clock::now() + timeout, handle });
        }

        // Resumes n times.
        void loop(std::size_t n = std::numeric_limits<std::size_t>::max())
        {
            std::size_t i = 0;

            while (!m_tasks.empty())
            {
                auto& timer = m_tasks.top();
                // if it is time to run a coroutine
                if (timer.targetTime < std::chrono::steady_clock::now())
                {
                    if (i++ < n)
                    {
                        auto handle = timer.handle;
                        m_tasks.pop();
                        handle.resume();
                    }
                    else
                    {
                        break;
                    }
                }
                else
                {
                    std::this_thread::sleep_until(timer.targetTime);
                }
            }
        }

        bool empty() const
        {
            return m_tasks.empty();
        }

        void clear()
        {
            while (!m_tasks.empty())
            {
                m_tasks.pop();
            }
        }

    private:

        struct Task
        {
            std::chrono::steady_clock::time_point targetTime;
            std::coroutine_handle<> handle;
        };

        using Compare = member_compare<&Task::targetTime>;

        std::priority_queue<Task, std::vector<Task>, Compare> m_tasks;
    };

    class TimeAwaitable
    {
    public:

        TimeAwaitable(TimeQueue& time_queue, std::chrono::nanoseconds d) :
            timeQueue(time_queue),
            m_d(d)
        {
        }

        bool await_ready()
        {
            return m_d <= std::chrono::nanoseconds(0);
        }

        // h is a handler for current coroutine which is suspended
        void await_suspend(std::coroutine_handle<> h)
        {
            // submit suspended coroutine to be resumed after timeout
            timeQueue.push(h, m_d);
        }

        void await_resume() {}

    private:

        TimeQueue& timeQueue;
        std::chrono::nanoseconds m_d;
    };

    inline awl::testing::TimeQueue timeQueue;

    inline TimeAwaitable operator co_await(std::chrono::nanoseconds d) noexcept
    {
        return TimeAwaitable(timeQueue, d);
    }
}