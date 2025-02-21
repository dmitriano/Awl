/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Testing/TestChain.h"

#include <iostream>

namespace awl 
{
    namespace testing 
    {
        //TODO: Implement QueryTests(filter) with std::ranges and move all output related code to TestConsole.
        class TestRunner
        {
        public:

            TestRunner(ostringstream& last_output);

            String GetLastOutput() const
            {
                return lastOutput.str();
            }

            void RunLink(const TestLink* p_test_link, const TestContext& context);

        private:

            class NullBuffer : public std::basic_streambuf<Char>
            {
            public:
                int overflow(int c) { return c; }
            };

            NullBuffer nullBuffer;
            std::basic_ostream<Char> nullOutput;

            ostringstream& lastOutput;
        };
    }
}
