#pragma once

#include "Awl/StringFormat.h"
#include "Awl/Testing/TestException.h"

namespace awl
{
    namespace testing
    {
        class Assert
        {
        public:
            
            [[noreturn]]
            static void Fail(const TCHAR * message = _T("Assertion failed."))
            {
                throw TestException(message);
            }

            static void IsTrue(bool val, const TCHAR * message = _T("The value is not true."))
            {
                if (!val)
                {
                    Fail(message);
                }
            }

            static void IsFalse(bool val, const TCHAR * message = _T("The value is not false."))
            {
                if (val)
                {
                    Fail(message);
                }
            }

            template <typename E, typename A>
            static void AreEqual(E expected, A actual, const TCHAR * message = _T("The values are not equal."))
            {
                if (expected != actual)
                {
                    throw TestException(format() << message << _T(" ") << _T(" expected ") << expected << _T(", actual ") << actual << _T("."));
                }
            }
        };
    }
}
