/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/SingleList.h"
#include "Awl/String.h"
#include "Awl/Testing/TestContext.h"

namespace awl
{
    namespace testing
    {
        using TestFunc = void(*)(const TestContext & context);

        //The objects of this class are static, they are not supposed to be created on the heap or on the stack.
        //There is no safe_exclude() in the destructor, because the order of static objects destruction in undefined,
        //so the object is not excluded from the list automatically before destruction.
        class TestLink : public single_link
        {
        public:

            TestLink(const Char * p_test_name, TestFunc p_test_func);

            const Char * GetName()
            {
                return pTestName;
            }

            void Run(const TestContext & context)
            {
                pTestFunc(context);
            }

        private:

            const Char * pTestName;

            TestFunc pTestFunc;
        };

        using TestChain = single_list<TestLink>;

        //No additional data structures are created when the program starts except the linked list of the static TestLink objects.
        inline TestChain & GetTestChain()
        {
            static TestChain testChain;

            return testChain;
        }

        inline TestLink::TestLink(const Char * p_test_name, TestFunc p_test_func) :
            pTestName(p_test_name), pTestFunc(p_test_func)
        {
            GetTestChain().push_front(this);
        }

        //Guarantees the clean process exit without dangling pointers. Call it at the end of main() function, for example.
        inline void Shutdown()
        {
            auto & chain = GetTestChain();

            while (!chain.empty())
            {
                chain.pop_front();
            }
        }
    }
}

#define AWT_LINK_FUNC_NAME(test_name) test_name##_TestFunc
#define AWT_LINK_FUNC_SIGNATURE(test_name) static void AWT_LINK_FUNC_NAME(test_name)(const awl::testing::TestContext & context)

//A test is simply a static function.
#define AWT_LINK(test_name, suffix) \
    AWT_LINK_FUNC_SIGNATURE(test_name); \
    static awl::testing::TestLink test_name##_##suffix_TestLink(_T(#test_name) _T("_") _T(#suffix), &AWT_LINK_FUNC_NAME(test_name)); \
    AWT_LINK_FUNC_SIGNATURE(test_name)

#define AWT_DISABLED_FUNC(test_name) \
    AWT_LINK_FUNC_SIGNATURE(test_name)

#define AWT_DISABLED_TEST(test_name) \
    AWT_DISABLED_FUNC(test_name)

#define AWT_DISABLED_BENCHMARK(test_name) \
    AWT_DISABLED_FUNC(test_name)

#define AWT_DISABLED_EXAMPLE(test_name) \
    AWT_DISABLED_FUNC(test_name)

#define AWT_TEST(test_name) AWT_LINK(test_name, Test)
#define AWT_BENCHMARK(test_name) AWT_LINK(test_name, Benchmark)
#define AWT_EXAMPLE(test_name) AWT_LINK(test_name, Example)

#define AWT_UNSTABLE_TEST(test_name) AWT_LINK(test_name, Test_Unstable)
#define AWT_UNSTABLE_BENCHMARK(test_name) AWT_LINK(test_name, Benchmark_Unstable)
#define AWT_UNSTABLE_EXAMPLE(test_name) AWT_LINK(test_name, Example_Unstable)

#define AWT_UNUSED_CONTEXT static_cast<void>(context)
