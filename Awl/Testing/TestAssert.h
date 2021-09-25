/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

        static void IsTrue(bool val, const Char* message = _T("The value is not true."))
        {
            if (!val)
            {
                Fail(message);
            }
        }

        static void IsFalse(bool val, const Char* message = _T("The value is not false."))
        {
            if (val)
            {
                Fail(message);
            }
        }

        template <typename E, typename A>
        static void AreEqual(E expected, A actual, const Char* message = _T("The values are not equal."))
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

#define AWT_STRINGIFY(x) #x
#define AWT_TOSTRING(x) AWT_STRINGIFY(x)
#define AWT_SRC_INFO _T(" ") __FILE__ _T(":") _T(AWT_TOSTRING(__LINE__))

#define AWT_FAIL awl::testing::Assert::Fail(AWT_SRC_INFO)
#define AWT_FAILM(message) awl::testing::Assert::Fail(_T(#message) AWT_SRC_INFO)

#ifndef AWT_NO_ASSERTS

#define AWT_ASSERT(cond) awl::testing::Assert::IsTrue(cond, _T(#cond) _T(" ") AWT_SRC_INFO)
#define AWT_ASSERTM(cond, message) awl::testing::Assert::IsTrue(cond, _T(#cond) _T(" ") message AWT_SRC_INFO)

#define AWT_ASSERT(cond) awl::testing::Assert::IsTrue(cond, _T(#cond) _T(" ") AWT_SRC_INFO)
#define AWT_ASSERTM_TRUE(cond, message) awl::testing::Assert::IsTrue(cond, _T(#cond) _T(" ") message AWT_SRC_INFO)

#define AWT_ASSERT_FALSE(cond) awl::testing::Assert::IsFalse(cond, _T(#cond) _T(" is not false. ") AWT_SRC_INFO)
#define AWT_ASSERTM_FALSE(cond, message) awl::testing::Assert::IsFalse(cond, _T(#cond) _T(" is not false. ") message AWT_SRC_INFO)

#define AWT_ASSERT_EQUAL(expected, actual) awl::testing::Assert::AreEqual(expected, actual, _T(#actual) _T(" != ") _T(#expected) AWT_SRC_INFO)
#define AWT_ASSERTM_EQUAL(expected, actual, message) awl::testing::Assert::AreEqual(expected, actual, _T(#actual) _T(" != ") _T(#expected) _T(" ") message AWT_SRC_INFO)

#else

#define AWT_FAKE_ASSERT ((void)0)

#define AWT_ASSERT(cond) AWT_FAKE_ASSERT
#define AWT_ASSERTM(cond, message) AWT_FAKE_ASSERT

#define AWT_ASSERT(cond) AWT_FAKE_ASSERT
#define AWT_ASSERTM_TRUE(cond, message) AWT_FAKE_ASSERT

#define AWT_ASSERT_FALSE(cond) AWT_FAKE_ASSERT
#define AWT_ASSERTM_FALSE(cond, message) AWT_FAKE_ASSERT

#define AWT_ASSERT_EQUAL(expected, actual) AWT_FAKE_ASSERT
#define AWT_ASSERTM_EQUAL(expected, actual, message) AWT_FAKE_ASSERT

#endif
