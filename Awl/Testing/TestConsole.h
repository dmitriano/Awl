/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/String.h"
#include "Awl/Testing/TestContext.h"
#include "Awl/Testing/CommandLineProvider.h"
#include "Awl/Testing/CompositeProvider.h"

#include "Awl/ConsoleLogger.h"
#include "Awl/Sleep.h"

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

        TestConsole(CompositeProvider<CommandLineProvider>& ap);

        int Run();

        const TestContext& context() const
        {
            return m_context;
        }

    private:

        bool RunTests();
            
        std::stop_source m_source;

        std::function<bool(const std::string& s)> CreateFilter(const std::string& filter);

        ConsoleLogger m_logger;

        CompositeProvider<CommandLineProvider>& m_ap;

        TestContext m_context;
    };

    int Run();

    int Run(int argc, Char* argv[]);
}
