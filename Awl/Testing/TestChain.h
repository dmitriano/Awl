#pragma once

#include "Awl/SingleList.h"
#include "Awl/String.h"
#include "Awl/Testing/TestContext.h"

namespace awl 
{
    namespace testing 
    {
        typedef void (*TestFunc)(const TestContext & context);
        
        //The objects of this class are static, they are not supposed to be created on the heap or on the stack.
        //There is no safe_exclude() in the destructor, because the order of static objects destruction in undefined,
        //so the object is not excluded from the list automatically before destruction.
        class TestLink : public single_link
        {
        public:

            TestLink(const Char * p_test_name, TestFunc p_test_func);

            String GetName()
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
        
        typedef single_list<TestLink> TestChain;

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

#define AWL_TEST_FUNC_NAME(test_name) test_name##_TestFunc
#define AWL_TEST_FUNC_SIGNATURE(test_name) static void AWL_TEST_FUNC_NAME(test_name)(const awl::testing::TestContext & context)

//A test is simply a static function.
#define AWL_TEST(test_name) \
    AWL_TEST_FUNC_SIGNATURE(test_name); \
    static awl::testing::TestLink test_name##_TestLink(_T(#test_name), &AWL_TEST_FUNC_NAME(test_name)); \
    AWL_TEST_FUNC_SIGNATURE(test_name)