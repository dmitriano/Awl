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

            void Run(const TestContext & context, const String & name)
            {
                auto i = testMap.find(name);

                assert(i != testMap.end());

                InternalRun(i->second, context);
            }
            
            void RunAll(const TestContext & context)
            {
                for (auto & p : testMap)
                {
                    InternalRun(p.second, context);
                }
            }

            String GetLastOutput() const
            {
                return lastOutput.str();
            }

        private:

            void InternalRun(TestLink * p_test_link, const TestContext & context)
            {
                context.out << p_test_link->GetName() << _T("...");

                const TestContext temp_context{ lastOutput, context.cancellation, context.ap };

                p_test_link->Run(temp_context);

                context.out << _T("OK") << std::endl;

                lastOutput.clear();
            }

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
