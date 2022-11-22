#pragma once

#include <thread>

#ifdef __APPLE__

    #include "Awl/CppStd/jthread.hpp"
    #include "Awl/CppStd/condition_variable_any2.hpp"

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
