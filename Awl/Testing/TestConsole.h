/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/String.h"
#include "Awl/Testing/TestContext.h"

#include "Awl/Cancellation.h"

#include <map>
#include <memory>
#include <iostream>
#include <functional>
#include <algorithm>
#include <assert.h>

namespace awl::testing
{
    class TestConsole
    {
    public:

        TestConsole() : m_cancellation(std::chrono::seconds(default_cancellation_timeout)) {}

        int RunAllTests();

        int Run(int argc, Char* argv[]);

    private:

        int RunTests(const TestContext& context);
            
        std::function<bool(const String& s)> CreateFilter(const String filter);
            
        TimedCancellationFlag m_cancellation;

        static const size_t default_cancellation_timeout = 5;
    };

    inline int Run(int argc, Char* argv[])
    {
        TestConsole console;

        return console.Run(argc, argv);
    }
}
