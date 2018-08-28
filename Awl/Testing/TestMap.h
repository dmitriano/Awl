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

            ostringstream lastOutput;
            
            typedef std::map<const Char *, TestLink *, CStringLess<Char>> Map;

            Map testMap;
        };

        inline std::shared_ptr<TestMap> CreateTestMap()
        {
            return std::make_shared<TestMap>();
        }

        int RunAllTests();
        
        int RunAllTests(const TestContext & context);

        int Run(int argc, Char * argv[]);
    }
}
