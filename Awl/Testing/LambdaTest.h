#pragma once

#include "Awl/Testing/TestChain.h"

#include "Awl/String.h"

#include <functional>

namespace awl 
{
    namespace testing 
    {
        class LambdaTest : public TestLink
        {
        public:

            LambdaTest(const Char * p_name, const std::function<void ()> & func) : pTestName(p_name), testFunc(func)
            {
            }

            String GetName() override
            {
                return pTestName;
            }

            void Run() override
            {
                testFunc();
            }

        private:

            const std::function<void()> testFunc;

            const Char * pTestName;
        };
    }
}

#define AWL_TEST(name, lambda_body) awl::testing::LambdaTest test##name(_T(#name), []() {lambda_body});

#define AWL_TEST_FUNC(func_name) awl::testing::LambdaTest test##func_name(_T(#func_name), func_name);
