/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/String.h"
#include "Awl/Sleep.h"
#include "Awl/Logger.h"
#include "Awl/StringFormat.h"
#include "Awl/Testing/CommandLineProvider.h"
#include "Awl/Testing/CompositeProvider.h"

namespace awl::testing
{
    template <class ... Ps>
    struct CompositeTestContext
    {
        // TODO: out was replaced with logger. Remove out.
        // A mutex can be used for synchronizing output operations in multithreaded tests, for example.
        awl::ostream& out;

        Logger& logger;

        std::stop_token stopToken;

        CompositeProvider<Ps...>& ap;
    };

    using CommandLineContext = CompositeTestContext<CommandLineProvider>;

    using TestContext = CommandLineContext;
}
