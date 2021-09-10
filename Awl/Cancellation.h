/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
            std::lock_guard lock(mutex);

            return isCancelled;
        }

        void InterruptibleSleep(std::chrono::nanoseconds time) const override
        {
            std::unique_lock lock(mutex);

            cv.wait_for(lock, time, [this]() -> bool
            {
                return isCancelled;
            });
        }

        void Cancel()
        {
            {
                std::lock_guard lock(mutex);

                isCancelled = true;
            }

            cv.notify_all();
        }

        void Reset()
        {
            std::lock_guard lock(mutex);

            isCancelled = false;
        }

    private:

        bool isCancelled;

        mutable std::mutex mutex;
        mutable std::condition_variable cv;
    };
}
