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

        virtual bool IsCancelled() const = 0;

        virtual void InterruptibleSleep(std::chrono::nanoseconds time) const = 0;

        template <class Rep, class Period>
        void Sleep(const std::chrono::duration<Rep, Period>& time) const
        {
            InterruptibleSleep(std::chrono::duration_cast<std::chrono::nanoseconds>(time));
        }
    };

    class CancellationFlag : public Cancellation
    {
    public:

        CancellationFlag() : isCancelled(false)
        {
        }

        bool IsCancelled() const override
        {
            return isCancelled;
        }

        void InterruptibleSleep(std::chrono::nanoseconds time) const override
        {
            std::unique_lock<std::mutex> lock(mutex);

            cv.wait_for(lock, time, [this]() -> bool
            {
                return isCancelled;
            });
        }

        void Cancel()
        {
            isCancelled = true;

            cv.notify_all();
        }

        void Reset()
        {
            isCancelled = false;
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
