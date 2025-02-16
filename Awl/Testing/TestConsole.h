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
    template <attribute_provider Provider>
    class TestConsole
    {
    public:

        explicit TestConsole(Provider& ap);

        int Run();

        const TestContext& context() const
        {
            return m_context;
        }

    private:

        bool RunTests();
            
        std::stop_source m_source;

        ConsoleLogger m_logger;

        Provider& m_ap;

        TestContext m_context;
    };

    // For acessing Command Line attributes without creating TestContext.
    template <attribute_provider Provider>
    struct ProviderContext
    {
        Provider& ap;
    };

    int Run();

    int Run(int argc, CmdChar* argv[]);
}
