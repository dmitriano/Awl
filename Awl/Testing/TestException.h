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

            explicit TestException() : theMessage(_T("No messsage provided."))
            {
            }

            explicit TestException(const String & message) : theMessage(message)
            {
            }

            String GetMessage() const override
            {
                return theMessage;
            }
        };
    }
}
