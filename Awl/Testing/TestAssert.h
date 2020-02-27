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

//a possible solution for escaping the comma
#define AWT_LIST(...) __VA_ARGS__

#define AWT_STRINGIFY(x) #x
#define AWT_TOSTRING(x) AWT_STRINGIFY(x)
#define AWT_SRC_INFO _T(" ") __FILE__ _T(":") _T(AWT_TOSTRING(__LINE__))

#define AWT_FAIL(message) awl::testing::Assert::Fail(_T(#message) AWT_SRC_INFO)

#define AWT_ASSERT_TRUE(cond) awl::testing::Assert::IsTrue(cond, _T(#cond) AWT_SRC_INFO)
#define AWT_ASSERT_IS_TRUE(cond, message) awl::testing::Assert::IsTrue(cond, _T(#cond) _T(" ") message AWT_SRC_INFO)

#define AWT_ASSERT_FALSE(cond) awl::testing::Assert::IsFalse(cond, _T(#cond) AWT_SRC_INFO)
#define AWT_ASSERT_IS_FALSE(cond, message) awl::testing::Assert::IsFalse(cond, _T(#cond) _T(" ") message AWT_SRC_INFO)

#define AWT_ASSERT_EQUAL(expected, actual) awl::testing::Assert::AreEqual(expected, actual, _T(#actual) _T(" != ") _T(#expected) AWT_SRC_INFO)
#define AWT_ASSERT_ARE_EQUAL(expected, actual, message) awl::testing::Assert::AreEqual(expected, actual, _T(#actual) _T(" != ") _T(#expected) _T(" ") message AWT_SRC_INFO)
