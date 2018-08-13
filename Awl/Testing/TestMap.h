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

            void Run(const String & name, ostream & out)
            {
                auto i = testMap.find(name);

                assert(i != testMap.end());

                InternalRun(i->second, out);
            }
            
            void RunAll(ostream & out)
            {
                for (auto & p : testMap)
                {
                    InternalRun(p.second, out);
                }
            }

            String GetLastOutput() const
            {
                return lastOutput.str();
            }

        private:

            void InternalRun(TestLink * p_test_link, ostream & out)
            {
                out << p_test_link->GetName() << _T("...");

                const TestContext test_context{ lastOutput, cancellationFlag };

                p_test_link->Run(test_context);

                out << _T("OK") << std::endl;

                lastOutput.clear();
            }

            ostringstream lastOutput;
            
            Cancellation cancellationFlag;
                
            typedef std::map<String, TestLink *> Map;

            Map testMap;
        };

        inline std::shared_ptr<TestMap> CreateTestMap()
        {
            return std::make_shared<TestMap>();
        }

        int RunAllTests(ostream & out);
    }
}
