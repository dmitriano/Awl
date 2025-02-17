/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Testing/TestChain.h"
#include "Awl/StaticMap.h"

#include <memory>
#include <iostream>
#include <functional>
#include <algorithm>
#include <assert.h>

namespace awl 
{
    namespace testing 
    {
        //TODO: Implement QueryTests(filter) with std::ranges and move all output related code to TestConsole.
        class TestRunner
        {
        public:

            TestRunner(ostringstream& last_output, const std::string& filter);

            void Run(const TestContext& context, const char * name);
            
            void RunAll(const TestContext& context);

            String GetLastOutput() const
            {
                return lastOutput.str();
            }

            size_t GetCount() const
            {
                return testMap.size();
            }

        private:

            void RunLink(const TestLink* p_test_link, const TestContext& context);

            class NullBuffer : public std::basic_streambuf<Char>
            {
            public:
                int overflow(int c) { return c; }
            };

            NullBuffer nullBuffer;
            std::basic_ostream<Char> nullOutput;

            ostringstream& lastOutput;
            
            StaticMap<TestFunc> testMap;
        };
    }
}
