/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/EquatableFunction.h"
#include "Awl/Testing/UnitTest.h"

#include <unordered_set>

namespace
{
    class Handler
    {
    public:

        void on_value(int value)
        {
            sum += value;
        }

        void on_other(int value)
        {
            sum += value * 2;
        }

        bool test_value(int value) const
        {
            return value > threshold;
        }

        int sum = 0;
        int threshold = 0;
    };
}

AWL_TEST(EquatableFunction_MemPtr)
{
    AWL_UNUSED_CONTEXT;

    Handler h1;
    Handler h2;

    AWL_ASSERT(&Handler::on_value == &Handler::on_value);
}

AWL_TEST(EquatableFunction_CompareAndHash)
{
    AWL_UNUSED_CONTEXT;

    Handler h1;
    Handler h2;

    awl::equatable_function<void(int)> f1(&h1, &Handler::on_value);
    awl::equatable_function<void(int)> f2(&h1, &Handler::on_value);
    awl::equatable_function<void(int)> f3(&h2, &Handler::on_value);
    awl::equatable_function<void(int)> f4(&h1, &Handler::on_other);

    AWL_ASSERT(f1 == f2);
    AWL_ASSERT_FALSE(f1 == f3);
    AWL_ASSERT_FALSE(f1 == f4);

    const auto hsh1 = std::hash<awl::equatable_function<void(int)>>{}(f1);
    const auto hsh2 = std::hash<awl::equatable_function<void(int)>>{}(f2);

    AWL_ASSERT_EQUAL(hsh1, hsh2);

    std::unordered_set<awl::equatable_function<void(int)>> set;
    AWL_ASSERT(set.insert(f1).second);
    AWL_ASSERT_FALSE(set.insert(f2).second);
    AWL_ASSERT(set.insert(f3).second);
    AWL_ASSERT(set.insert(f4).second);

    AWL_ASSERT_EQUAL(3u, set.size());
}

AWL_TEST(EquatableFunction_Invoke)
{
    AWL_UNUSED_CONTEXT;

    Handler h;

    awl::equatable_function<void(int)> f(&h, &Handler::on_value);

    AWL_ASSERT(static_cast<bool>(f));
    f(5);
    f(7);

    AWL_ASSERT_EQUAL(12, h.sum);
}

AWL_TEST(EquatableFunction_UnorderedSet)
{
    AWL_UNUSED_CONTEXT;

    Handler h1;
    Handler h2;

    awl::equatable_function<void(int)> f1(&h1, &Handler::on_value);
    awl::equatable_function<void(int)> f2(&h1, &Handler::on_value);
    awl::equatable_function<void(int)> f3(&h2, &Handler::on_value);

    std::unordered_set<awl::equatable_function<void(int)>> handlers;
    AWL_ASSERT(handlers.insert(f1).second);
    AWL_ASSERT_FALSE(handlers.insert(f2).second);
    AWL_ASSERT(handlers.insert(f3).second);

    AWL_ASSERT_EQUAL(2u, handlers.size());
    AWL_ASSERT(handlers.find(f1) != handlers.end());
    AWL_ASSERT(handlers.find(f2) != handlers.end());
    AWL_ASSERT(handlers.find(f3) != handlers.end());
}

AWL_TEST(EquatableFunction_ConstMember)
{
    AWL_UNUSED_CONTEXT;

    Handler h;
    h.threshold = 10;

    const Handler* p_h = &h;
    awl::equatable_function<bool(int)> f(p_h, &Handler::test_value);

    AWL_ASSERT(f(11));
    AWL_ASSERT_FALSE(f(8));
}

AWL_TEST(EquatableFunction_EmptyCompare)
{
    AWL_UNUSED_CONTEXT;

    awl::equatable_function<void()> f1;
    awl::equatable_function<void()> f2;

    AWL_ASSERT(f1 == f2);

    const auto hsh1 = std::hash<awl::equatable_function<void()>>{}(f1);
    const auto hsh2 = std::hash<awl::equatable_function<void()>>{}(f2);

    AWL_ASSERT_EQUAL(hsh1, hsh2);
}
