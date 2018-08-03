#pragma once

#include "Awl/String.h"

#include <atomic>
#include <mutex>

namespace awl 
{
    namespace testing 
    {
        struct TestCancellation
        {
            virtual bool IsCancelled() = 0;
        };
        
        class CancellationFlag : public TestCancellation
        {
        public:

            CancellationFlag() : isCancelled(false)
            {
            }

            bool IsCancelled() override
            {
                return isCancelled;
            }

            CancellationFlag & operator = (bool val)
            {
                isCancelled = val;

                return *this;
            }

        private:

            std::atomic<bool> isCancelled;
        };

        struct TestContext
        {
            //std::recursive_mutex & mutex;
            
            std::basic_ostream<Char> & out;

            TestCancellation & cancellation;
        };
    }
}
