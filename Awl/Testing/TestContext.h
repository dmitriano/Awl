/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/String.h"
#include "Awl/Sleep.h"
#include "Awl/Testing/AttributeProvider.h"

#include <atomic>
#include <condition_variable>
#include <chrono>

namespace awl 
{
    namespace testing 
    {
        struct TestContext
        {
            //A mutex can be used for synchronizing output operations in multithreaded tests, for example.
            awl::ostream& out;

            std::stop_token stopToken;

            AttributeProvider& ap;
        };
    }
}
