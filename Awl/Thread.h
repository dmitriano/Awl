#pragma once

#include <thread>

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
