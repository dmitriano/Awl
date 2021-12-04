/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/StopWatch.h"

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
            std::lock_guard lock(m_mutex);

            return isCancelled;
        }

        void InterruptibleSleep(std::chrono::nanoseconds time) const override
        {
            std::unique_lock lock(m_mutex);

            m_cv.wait_for(lock, time, [this]() -> bool
            {
                return isCancelled;
            });
        }

        void Cancel()
        {
            {
                std::lock_guard lock(m_mutex);

                isCancelled = true;
            }

            m_cv.notify_all();
        }

        virtual void Reset()
        {
            std::lock_guard lock(m_mutex);

            isCancelled = false;
        }

    protected:

        bool isCancelled;

        mutable std::mutex m_mutex;

    private:

        mutable std::condition_variable m_cv;
    };

    class TimedCancellationFlag : public CancellationFlag
    {
    public:

        TimedCancellationFlag(std::chrono::nanoseconds d) : m_d(d) {}

        bool IsCancelled() const override
        {
            std::lock_guard lock(m_mutex);

            return isCancelled || m_sw.HasElapsed(m_d);
        }

        void Reset() override
        {
            std::lock_guard lock(m_mutex);

            isCancelled = false;
            m_sw.Reset();
        }

        void SetTimeout(std::chrono::nanoseconds d)
        {
            std::lock_guard lock(m_mutex);

            m_d = d;
        }

    private:

        awl::StopWatch m_sw;
        std::chrono::nanoseconds m_d;
    };
}
