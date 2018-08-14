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

            TestMap()
            {
                for (TestLink * p_link : GetTestChain())
                {
                    testMap.emplace(p_link->GetName(), p_link);
                }
            }

            void Run(const TestContext & context, const String & name);
            
            void RunAll(const TestContext & context);

            String GetLastOutput() const
            {
                return lastOutput.str();
            }

        private:

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
