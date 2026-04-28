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
        static void fail(const String message = _T("An assertion failed."))
        {
            throw TestException(message);
        }

        static void isTrue(bool val, const Char* message = _T("The value is not true."))
        {
            if (!val)
            {
                fail(message);
            }
        }

        static void isFalse(bool val, const Char* message = _T("The value is not false."))
        {
            if (val)
            {
                fail(message);
            }
        }

        template <typename E, typename A>
        static void areEqual(E expected, A actual, const Char* message = _T("The values are not equal."))
        {
            if (expected != actual)
            {
                awl::ostringstream out;
                out << message << _T(" ") << _T(" expected ") << expected << _T(", actual ") << actual << _T(".");
                fail(out.str());
            }
        }

        template <class E, class Func>
        static void throws(Func&& func)
        {
            try
            {
                func();
                Assert::fail(std::format(_T("Exception of type '{}' was not thrown."), fromACString(typeid(E).name())));
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

#define AWL_FAIL awl::testing::Assert::fail(AWL_SRC_INFO)
#define AWL_FAILM(message) awl::testing::Assert::fail(_T(#message) AWL_SRC_INFO)

#ifndef AWL_NO_ASSERTS

#define AWL_ASSERT(cond) awl::testing::Assert::isTrue(cond, _T(#cond) _T(" ") AWL_SRC_INFO)
#define AWL_ASSERTM(cond, message) awl::testing::Assert::isTrue(cond, _T(#cond) _T(" ") message AWL_SRC_INFO)

#define AWL_ASSERT(cond) awl::testing::Assert::isTrue(cond, _T(#cond) _T(" ") AWL_SRC_INFO)
#define AWL_ASSERTM_TRUE(cond, message) awl::testing::Assert::isTrue(cond, _T(#cond) _T(" ") message AWL_SRC_INFO)

#define AWL_ASSERT_FALSE(cond) awl::testing::Assert::isFalse(cond, _T(#cond) _T(" is not false. ") AWL_SRC_INFO)
#define AWL_ASSERTM_FALSE(cond, message) awl::testing::Assert::isFalse(cond, _T(#cond) _T(" is not false. ") message AWL_SRC_INFO)

#define AWL_ASSERT_EQUAL(expected, actual) awl::testing::Assert::areEqual(expected, actual, _T(#actual) _T(" != ") _T(#expected) AWL_SRC_INFO)
#define AWL_ASSERTM_EQUAL(expected, actual, message) awl::testing::Assert::areEqual(expected, actual, _T(#actual) _T(" != ") _T(#expected) _T(" ") message AWL_SRC_INFO)

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
