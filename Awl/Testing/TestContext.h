/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/String.h"
#include "Awl/Sleep.h"
#include "Awl/Testing/CommandLineProvider.h"
#include "Awl/Testing/CompositeProvider.h"

#include <atomic>
#include <condition_variable>
#include <chrono>

namespace awl::testing
{
    template <class ... Ps>
    struct CompositeTestContext
    {
        //A mutex can be used for synchronizing output operations in multithreaded tests, for example.
        awl::ostream& out;

        std::stop_token stopToken;

        CompositeProvider<Ps...>& ap;
    };

    using TestContext = CompositeTestContext<CommandLineProvider>;
}
