#pragma once

#include <thread>
#include <condition_variable>
#include <stop_token>

//defined(__APPLE__) || defined(__ANDROID__)
#ifdef AWL_JTHREAD_EXTRAS

    #include "JThreadExtras/jthread.hpp"
    #include "JThreadExtras/condition_variable_any2.hpp"

    namespace awl
    {
        using condition_variable_any = std::condition_variable_any2;
    }

#else

    namespace awl
    {
        using condition_variable_any = std::condition_variable_any;
    }

#endif
