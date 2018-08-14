#pragma once

#include "Awl/String.h"

namespace awl
{
    namespace testing
    {
        class TestException
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

            const String & GetMessage() const
            {
                return theMessage;
            }
        };
    }
}
