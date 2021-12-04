/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/String.h"
#include "Awl/Testing/TestContext.h"
#include "Awl/Testing/AttributeProvider.h"

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

        TestConsole(AttributeProvider& ap) : m_ap(ap) {}

        int Run();

    private:

        int RunTests(const TestContext& context);
            
        std::function<bool(const String& s)> CreateFilter(const String filter);

        AttributeProvider m_ap;
            
        CancellationFlag m_cancellation;
    };

    int Run();

    int Run(int argc, Char* argv[]);
}
