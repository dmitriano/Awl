/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <chrono>
#include <queue>
#include <vector>
#include <thread>
#include <coroutine>

namespace awl::testing
{
    class TimeQueue
    {
    public:

        void push(std::coroutine_handle<> handle, std::chrono::nanoseconds timeout)
        {
            m_timers.push(Task{ std::chrono::steady_clock::now() + timeout, handle });
        }

        void loop()
        {
            while (!m_timers.empty())
            {
                auto& timer = m_timers.top();
                // if it is time to run a coroutine
                if (timer.targetTime < std::chrono::steady_clock::now())
                {
                    auto handle = timer.handle;
                    m_timers.pop();
                    handle.resume();
                }
                else
                {
                    std::this_thread::sleep_until(timer.targetTime);
                }
            }
        }

    private:

        struct Task
        {
            std::chrono::steady_clock::time_point targetTime;
            std::coroutine_handle<> handle;
        };

        // comparator
        struct timer_task_before_cmp
        {
            bool operator()(const Task& left, const Task& right) const
            {
                return left.targetTime > right.targetTime;
            }
        };

        std::priority_queue<Task, std::vector<Task>, timer_task_before_cmp> m_timers;
    };
}