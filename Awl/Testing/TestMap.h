/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Testing/TestChain.h"

#include <map>
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
        class TestMap
        {
        public:

            TestMap();

            void Run(const TestContext & context, const Char * name);
            
            void RunAll(const TestContext & context, const std::function<bool (const String&)> & filter);

            void PrintNames(awl::ostream & out, const std::function<bool(const String&)> & filter) const;

            String GetLastOutput() const
            {
                return lastOutput.str();
            }

            size_t GetTestCount() const
            {
                return testMap.size();
            }

            size_t GetCount(const std::function<bool(const String&)> & filter) const
            {
                return std::count_if(testMap.begin(), testMap.end(), [&filter](const std::pair<String, TestLink *> & p) { return filter(p.first); });
            }

        private:

            void InternalRun(TestLink * p_test_link, const TestContext & context);

            class NullBuffer : public std::basic_streambuf<Char>
            {
            public:
                int overflow(int c) { return c; }
            };

            NullBuffer nullBuffer;
            std::basic_ostream<Char> nullOutput;

            ostringstream lastOutput;
            
            using Map = std::map<const Char *, TestLink *, CStringLess<Char>>;

            Map testMap;
        };

        inline std::shared_ptr<TestMap> CreateTestMap()
        {
            return std::make_shared<TestMap>();
        }
    }
}
