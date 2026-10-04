/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/String.h"
#include "Awl/Testing/TestContext.h"
#include "Awl/Testing/CommandLineProvider.h"
#include "Awl/Testing/CompositeProvider.h"
#include "Awl/ILogger.h"
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

        awl::ostream& _outputStream;

        std::shared_ptr<ILogger> _logger;

        Provider& _ap;
        TypeProvider _typeProvider;

        TestContext _context;
    };

    int run();

    int run(std::stop_token stop_token);

    int run(int argc, CmdChar* argv[]);

    int run(int argc, CmdChar* argv[], std::stop_token stop_token);

    // Select a diagnostic stream even when command-line parsing fails.
    awl::ostream& commandLineOutputStream(int argc, CmdChar* argv[]);
}
