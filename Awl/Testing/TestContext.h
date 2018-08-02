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
            OutputStream & out;

            TestCancellation & cancellation;
        };
    }
}
