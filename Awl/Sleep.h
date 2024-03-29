/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <mutex>
#include <condition_variable>
#include <chrono>

#include "Awl/CppStd/Thread.h"

namespace awl
{
    template<typename Rep, typename Period>
    void sleep_for(const std::chrono::duration<Rep, Period>& d, const std::stop_token& token)
    {
        std::condition_variable cv;

        std::mutex mutex;

        std::unique_lock<std::mutex> lock{ mutex };

        // ThreadSanitizer: double lock of a mutex
        // std::condition_variable_any().wait_for(lock, token, d, [&token]
        // {
        //     return token.stop_requested();
        // });

        std::stop_callback stop_wait
        {
            token,
            [&cv]()
            {
                cv.notify_one();
            }
        };

        cv.wait_for(lock, d, [&token]()
        {
            return token.stop_requested();
        });
    }
}
