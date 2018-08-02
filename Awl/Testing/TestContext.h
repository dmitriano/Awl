#pragma once

#include "Awl/Testing/OutputStream.h"

#include <atomic>

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

            std::atomic<bool> isCancelled = false;
        };

        struct TestContext
        {
            OutputStream & out;

            TestCancellation & cancellation;
        };
    }
}
