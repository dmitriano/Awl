#pragma once

#include "Awl/Coro/IExecutor.h"

#include <condition_variable>
#include <deque>
#include <functional>
#include <mutex>
#include <thread>

namespace awl::testing
{
    class TestDispatcher : public awl::coro::IDispatcher
    {
    public:

        TestDispatcher() :
            _thread([this]
                {
                    run();
                })
        {}

        ~TestDispatcher() override
        {
            {
                std::lock_guard lock(_mutex);
                _stopped = true;
            }

            _condition.notify_all();

            if (_thread.joinable())
            {
                _thread.join();
            }
        }

        void post(std::move_only_function<void()> func) override
        {
            {
                std::lock_guard lock(_mutex);
                _queue.push_back(std::move(func));
            }

            _condition.notify_one();
        }

        void join() override
        {
            std::unique_lock lock(_mutex);

            _idle.wait(lock, [this]
                {
                    return _queue.empty() && !_running;
                });
        }

        bool empty() const
        {
            std::lock_guard lock(_mutex);

            return _queue.empty();
        }

    private:

        void run()
        {
            for (;;)
            {
                std::move_only_function<void()> func;

                {
                    std::unique_lock lock(_mutex);

                    _condition.wait(lock, [this]
                        {
                            return _stopped || !_queue.empty();
                        });

                    if (_stopped && _queue.empty())
                    {
                        break;
                    }

                    func = std::move(_queue.front());
                    _queue.pop_front();
                    _running = true;
                }

                func();

                {
                    std::lock_guard lock(_mutex);
                    _running = false;
                }

                _idle.notify_all();
            }
        }

        mutable std::mutex _mutex;
        std::condition_variable _condition;
        std::condition_variable _idle;
        std::deque<std::move_only_function<void()>> _queue;
        bool _stopped = false;
        bool _running = false;
        std::thread _thread;
    };
}
