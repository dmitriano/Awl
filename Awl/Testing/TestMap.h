#pragma once

#include "Awl/Testing/TestChain.h"

#include <map>
#include <memory>
#include <iostream>
#include <assert.h>

namespace awl 
{
    namespace testing 
    {
        class TestMap
        {
        public:

            TestMap();

            void Run(const TestContext & context, const String & name);
            
            void RunAll(const TestContext & context);

            String GetLastOutput() const
            {
                return lastOutput.str();
            }

            size_t GetTestCount() const
            {
                return testMap.size();
            }

        private:

            std::basic_ostream<Char> & GetOutputStream(const TestContext & context, bool verbose)
            {
                return verbose ? context.out : lastOutput;
            }

            void InternalRun(TestLink * p_test_link, const TestContext & context);

            ostringstream lastOutput;
            
            typedef std::map<String, TestLink *> Map;

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
