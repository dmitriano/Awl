#pragma once

#include "Awl/StringFormat.h"
#include "Awl/Testing/TestException.h"
#include "Awl/Exception.h"

#include <typeinfo>

namespace awl::testing
{
    class Assert
    {
    public:

        [[noreturn]]
        static void Fail(const String message = _T("An assertion failed."))
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
                Fail(format() << message << _T(" ") << _T(" expected ") << expected << _T(", actual ") << actual << _T("."));
            }
        }

        template <class E, class Func>
        static void Throws(Func && func)
        {
            try
            {
                func();
                Assert::Fail(format() << _T("Exception of type '") << FromACString(typeid(E).name()) << _T("' was not thrown."));
            }
            catch (const E &)
            {
            }
        }
    };
}

#define AWT_ASSERT_TRUE(cond) if (context.checkAsserts) awl::testing::Assert::IsTrue(cond, _T(#cond))
#define AWT_ASSERT_FALSE(cond) if (context.checkAsserts) awl::testing::Assert::IsFalse(cond, _T(#cond))
#define AWT_ASSERT_EQUAL(expected, actual) if (context.checkAsserts) awl::testing::Assert::AreEqual(expected, actual, _T(#actual) _T(" != ") _T(#expected))
