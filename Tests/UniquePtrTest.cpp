#include "Awl/UniquePtr.h"

#include "Awl/Testing/UnitTest.h"
#include "Awl/String.h"

#include <functional>

using namespace awl::testing;

namespace
{
    static int count = 0;
    
    struct A
    {
        A()
        {
            ++count;
        }

        std::function<void()> func;

        awl::String val;

        ~A()
        {
            func();

            --count;
        }
    };
}

//The similar code with std::unique_ptr results in a segmentation fault.
AWT_TEST(UniquePtrBegingDestoyed)
{
    AWT_UNUSED_CONTEXT;

    awl::unique_ptr<A> p_a = awl::make_unique<A>();

    AWT_ASSERT_EQUAL(1, count);

    p_a->val = _T("beging destoyed");
    p_a->func = [&p_a, context]() { context.out << p_a->val; };
    p_a = {};

    AWT_ASSERT_EQUAL(0, count);
}
