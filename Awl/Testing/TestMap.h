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

            void Run(const String & name)
            {
                auto i = testMap.find(name);

                assert(i != testMap.end());

                InternalRun(i->second);
            }
            
            void RunAll()
            {
                for (auto & p : testMap)
                {
                    InternalRun(p.second);
                }
            }

        private:

            void InternalRun(TestLink * p_test_link)
            {
                std::cout << p_test_link->GetName() << "...";

                p_test_link->Run();

                std::cout << "OK" << std::endl;
            }

            typedef std::map<String, TestLink *> Map;

            Map testMap;
        };

        inline std::shared_ptr<TestMap> CreateTestMap()
        {
            return std::make_shared<TestMap>();
        }
    }
}
