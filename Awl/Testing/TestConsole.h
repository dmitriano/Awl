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
#include <cassert>

namespace awl::testing
{
    template <attribute_provider Provider>
    class TestConsole
    {
    public:

        explicit TestConsole(Provider& provider, std::stop_token stop_token);

        int run();

        const TestContext& context() const
        {
            return _context;
        }

    private:

        bool runTests();
            
        std::shared_ptr<ConsoleLogger> _logger;

        Provider& _ap;
        TypeProvider _typeProvider;

        TestContext _context;
    };

    int run();

    int run(std::stop_token stop_token);

    int run(int argc, CmdChar* argv[]);

    int run(int argc, CmdChar* argv[], std::stop_token stop_token);
}
