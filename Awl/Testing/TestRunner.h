/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Testing/TestChain.h"

#include <functional>
#include <memory>

namespace awl 
{
    namespace testing 
    {
        class TestRunner
        {
        public:

            TestRunner() = default;

            TestRunner(std::function<void()> delay_output, std::function<void()> clear_output);

            void runLink(
                const TestLink* p_test_link,
                const TestContext& context,
                std::shared_ptr<ILogger> test_logger);

        private:

            std::function<void()> _delayOutput;
            std::function<void()> _clearOutput;
        };
    }
}
