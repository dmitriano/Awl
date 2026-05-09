/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Testing/TestChain.h"

namespace awl 
{
    namespace testing 
    {
        class TestRunner
        {
        public:

            TestRunner(ostringstream& last_output);

            String getLastOutput() const
            {
                return lastOutput.str();
            }

            void runLink(const TestLink* p_test_link, const TestContext& context);

        private:

            ostringstream& lastOutput;
        };
    }
}
