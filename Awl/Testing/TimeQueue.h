/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Coro/DelayedAwaitable.h"
#include "Awl/KeyCompare.h"

#include <queue>

#include <chrono>
#include <vector>
#include <thread>
#include <coroutine>
#include <limits>

namespace awl::testing
{
    class TimeQueue : public awl::coro::IDelayedExecutor
    {
    public:

        void executeAfter(std::coroutine_handle<> handle, std::chrono::nanoseconds delay) override
        {
            push(handle, delay);
        }

        void push(std::coroutine_handle<> handle, std::chrono::nanoseconds timeout)
        {
            _tasks.push(Task{ std::chrono::steady_clock::now() + timeout, handle });
        }

        // Resumes n times.
        void loop(std::size_t n = std::numeric_limits<std::size_t>::max())
        {
            std::size_t i = 0;

            while (!_tasks.empty())
            {
                auto& timer = _tasks.top();
                // if it is time to run a coroutine
                if (timer.targetTime < std::chrono::steady_clock::now())
                {
                    if (i++ < n)
                    {
                        auto handle = timer.handle;
                        _tasks.pop();
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
            return _tasks.empty();
        }

        void clear()
        {
            while (!_tasks.empty())
            {
                _tasks.pop();
            }
        }

    private:

        struct Task
        {
            std::chrono::steady_clock::time_point targetTime;
            std::coroutine_handle<> handle;
        };

        using Compare = member_compare<&Task::targetTime>;

        std::priority_queue<Task, std::vector<Task>, Compare> _tasks;
    };
}
