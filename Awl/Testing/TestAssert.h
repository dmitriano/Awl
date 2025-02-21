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

#define AWL_STRINGIFY(x) #x
#define AWL_TOSTRING(x) AWL_STRINGIFY(x)
#define AWL_SRC_INFO _T(" ") __FILE__ _T(":") _T(AWL_TOSTRING(__LINE__))

#define AWL_FAIL awl::testing::Assert::Fail(AWL_SRC_INFO)
#define AWL_FAILM(message) awl::testing::Assert::Fail(_T(#message) AWL_SRC_INFO)

#ifndef AWL_NO_ASSERTS

#define AWL_ASSERT(cond) awl::testing::Assert::IsTrue(cond, _T(#cond) _T(" ") AWL_SRC_INFO)
#define AWL_ASSERTM(cond, message) awl::testing::Assert::IsTrue(cond, _T(#cond) _T(" ") message AWL_SRC_INFO)

#define AWL_ASSERT(cond) awl::testing::Assert::IsTrue(cond, _T(#cond) _T(" ") AWL_SRC_INFO)
#define AWL_ASSERTM_TRUE(cond, message) awl::testing::Assert::IsTrue(cond, _T(#cond) _T(" ") message AWL_SRC_INFO)

#define AWL_ASSERT_FALSE(cond) awl::testing::Assert::IsFalse(cond, _T(#cond) _T(" is not false. ") AWL_SRC_INFO)
#define AWL_ASSERTM_FALSE(cond, message) awl::testing::Assert::IsFalse(cond, _T(#cond) _T(" is not false. ") message AWL_SRC_INFO)

#define AWL_ASSERT_EQUAL(expected, actual) awl::testing::Assert::AreEqual(expected, actual, _T(#actual) _T(" != ") _T(#expected) AWL_SRC_INFO)
#define AWL_ASSERTM_EQUAL(expected, actual, message) awl::testing::Assert::AreEqual(expected, actual, _T(#actual) _T(" != ") _T(#expected) _T(" ") message AWL_SRC_INFO)

#else

#define AWL_FAKE_ASSERT ((void)0)

#define AWL_ASSERT(cond) AWL_FAKE_ASSERT
#define AWL_ASSERTM(cond, message) AWL_FAKE_ASSERT

#define AWL_ASSERT(cond) AWL_FAKE_ASSERT
#define AWL_ASSERTM_TRUE(cond, message) AWL_FAKE_ASSERT

#define AWL_ASSERT_FALSE(cond) AWL_FAKE_ASSERT
#define AWL_ASSERTM_FALSE(cond, message) AWL_FAKE_ASSERT

#define AWL_ASSERT_EQUAL(expected, actual) AWL_FAKE_ASSERT
#define AWL_ASSERTM_EQUAL(expected, actual, message) AWL_FAKE_ASSERT

#endif
