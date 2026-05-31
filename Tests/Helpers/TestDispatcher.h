#pragma once

#include "Awl/Coro/IExecutor.h"

#include <deque>
#include <functional>
#include <mutex>

namespace awl::testing
{
    class TestDispatcher : public awl::coro::IDispatcher
    {
    public:

        void post(std::move_only_function<void()> func) override
        {
            std::lock_guard lock(_mutex);
            _queue.push_back(std::move(func));
        }

        void join() override
        {
            for (;;)
            {
                std::move_only_function<void()> func;

                {
                    std::lock_guard lock(_mutex);

                    if (_queue.empty())
                    {
                        break;
                    }

                    func = std::move(_queue.front());
                    _queue.pop_front();
                }

                func();
            }
        }

        bool empty() const
        {
            std::lock_guard lock(_mutex);

            return _queue.empty();
        }

    private:

        mutable std::mutex _mutex;
        std::deque<std::move_only_function<void()>> _queue;
    };
}
