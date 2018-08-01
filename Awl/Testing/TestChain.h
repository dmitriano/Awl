#pragma once

#include "Awl/QuickList.h"
#include "Awl/String.h"

namespace awl 
{
    namespace testing 
    {
        //The objects of this class are static, they are not supposed to be created on the heap or on the stack.
        //There is no safe_exclude() in the destructor, because the order of static objects destruction in undefined,
        //so the object is not excluded from the list automatically before destruction.
        class TestLink : public quick_link
        {
        protected:

            TestLink();

        public:

            virtual String GetName() = 0;

            virtual void Run() = 0;
        };
        
        typedef quick_list<TestLink> TestChain;

        inline TestChain & GetTestChain()
        {
            static TestChain testChain;
            
            return testChain;
        }

        inline TestLink::TestLink()
        {
            GetTestChain().push_back(this);
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
