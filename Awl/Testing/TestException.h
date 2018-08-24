#pragma once

#include "Awl/Exception.h"

namespace awl
{
    namespace testing
    {
        class TestException : public Exception
        {
        private:

            const String theMessage;

        public:

            explicit TestException(const String & message) : theMessage(message)
            {
            }

            String GetMessage() const override
            {
                return theMessage;
            }

            AWL_IMPLEMENT_EXCEPTION
        };
    }
}
