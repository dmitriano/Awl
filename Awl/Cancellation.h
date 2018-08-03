#pragma once

#include <atomic>
#include <mutex>
#include <condition_variable>
#include <chrono>

namespace awl 
{
    class Cancellation
    {
    public:

        Cancellation() : isCancelled(false)
        {
        }

        bool IsCancelled() const
        {
            return isCancelled;
        }

        void Cancel()
        {
            isCancelled = true;

            cv.notify_all();
        }

        template< class Rep, class Period >
        void Sleep(const std::chrono::duration<Rep, Period>& time) const
        {
            cv.wait_for(std::unique_lock<std::mutex>(mutex), time, [this]() -> bool
            {
                return isCancelled;
            });
        }

    private:

        std::atomic<bool> isCancelled;

        mutable std::mutex mutex;
        mutable std::condition_variable cv;

        //std::mutex introduces unnecessary overhead, but I am to sure it is safe to use std::condition_variable_any with a fake lock, but theoretically there can be:

        //struct fake_lock
        //{
        //    void lock() {}
        //    void unlock() {}
        //};

        //std::condition_variable_any cv;
    };
}
