/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/EquatableFunction.h"
#include "Awl/Testing/UnitTest.h"

#include <unordered_set>
#include <utility>

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

    class OtherHandler
    {
    public:

        void on_value(int value)
        {
            sum += value;
        }

        bool test_value(int value) const
        {
            return value > threshold;
        }

        int sum = 0;
        int threshold = 0;
    };

    class DerivedHandler : public Handler
    {
    public:

        void on_value_derived(int value)
        {
            sum += value * 3;
        }

        bool test_value_derived(int value) const
        {
            return value >= threshold;
        }
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
    OtherHandler other;
    DerivedHandler derived;

    awl::equatable_function<void(int)> f1(&h1, &Handler::on_value);
    awl::equatable_function<void(int)> f2(&h1, &Handler::on_value);
    awl::equatable_function<void(int)> f3(&h2, &Handler::on_value);
    awl::equatable_function<void(int)> f4(&h1, &Handler::on_other);
    awl::equatable_function<void(int)> f5 = f1;
    awl::equatable_function<void(int)> movable(&h1, &Handler::on_value);
    awl::equatable_function<void(int)> f6(std::move(movable));
    awl::equatable_function<void(int)> empty1;
    awl::equatable_function<void(int)> empty2;
    awl::equatable_function<void(int)> f_other(&other, &OtherHandler::on_value);
    awl::equatable_function<void(int)> f_derived(&derived, &DerivedHandler::on_value_derived);
    awl::equatable_function<void(int)> f_derived_dup(&derived, &DerivedHandler::on_value_derived);

    std::unordered_set<awl::equatable_function<void(int)>> handlers;
    AWL_ASSERT(handlers.insert(f1).second);
    AWL_ASSERT_FALSE(handlers.insert(f2).second);
    AWL_ASSERT(handlers.insert(f3).second);
    AWL_ASSERT(handlers.insert(f4).second);
    AWL_ASSERT_FALSE(handlers.insert(f5).second);
    AWL_ASSERT_FALSE(handlers.insert(f6).second);
    AWL_ASSERT(handlers.insert(empty1).second);
    AWL_ASSERT_FALSE(handlers.insert(empty2).second);
    AWL_ASSERT(handlers.insert(f_other).second);
    AWL_ASSERT(handlers.insert(f_derived).second);
    AWL_ASSERT_FALSE(handlers.insert(f_derived_dup).second);

    AWL_ASSERT_EQUAL(6u, handlers.size());
    AWL_ASSERT(handlers.find(f1) != handlers.end());
    AWL_ASSERT(handlers.find(f2) != handlers.end());
    AWL_ASSERT(handlers.find(f3) != handlers.end());
    AWL_ASSERT(handlers.find(f4) != handlers.end());
    AWL_ASSERT(handlers.find(empty1) != handlers.end());
    AWL_ASSERT(handlers.find(f_other) != handlers.end());
    AWL_ASSERT(handlers.find(f_derived) != handlers.end());
    AWL_ASSERT_EQUAL(1u, handlers.erase(f3));
    AWL_ASSERT(handlers.find(f3) == handlers.end());
    AWL_ASSERT_EQUAL(5u, handlers.size());

    awl::equatable_function<bool(int)> p1(&h1, &Handler::test_value);
    awl::equatable_function<bool(int)> p2(&h1, &Handler::test_value);
    awl::equatable_function<bool(int)> p3(&h2, &Handler::test_value);
    awl::equatable_function<bool(int)> p_other(&other, &OtherHandler::test_value);
    awl::equatable_function<bool(int)> p_derived(&derived, &DerivedHandler::test_value_derived);
    awl::equatable_function<bool(int)> p_derived_dup(&derived, &DerivedHandler::test_value_derived);

    std::unordered_set<awl::equatable_function<bool(int)>> predicates;
    AWL_ASSERT(predicates.insert(p1).second);
    AWL_ASSERT_FALSE(predicates.insert(p2).second);
    AWL_ASSERT(predicates.insert(p3).second);
    AWL_ASSERT(predicates.insert(p_other).second);
    AWL_ASSERT(predicates.insert(p_derived).second);
    AWL_ASSERT_FALSE(predicates.insert(p_derived_dup).second);
    AWL_ASSERT_EQUAL(4u, predicates.size());
    AWL_ASSERT(predicates.find(p2) != predicates.end());
    AWL_ASSERT(predicates.find(p_other) != predicates.end());
    AWL_ASSERT(predicates.find(p_derived) != predicates.end());
}

AWL_TEST(EquatableFunction_DifferentHandlerTypes)
{
    AWL_UNUSED_CONTEXT;

    Handler base;
    OtherHandler other;
    DerivedHandler derived1;
    DerivedHandler derived2;

    awl::equatable_function<void(int)> f_base(&base, &Handler::on_value);
    awl::equatable_function<void(int)> f_other(&other, &OtherHandler::on_value);
    awl::equatable_function<void(int)> f_derived1(&derived1, &DerivedHandler::on_value_derived);
    awl::equatable_function<void(int)> f_derived1_dup(&derived1, &DerivedHandler::on_value_derived);
    awl::equatable_function<void(int)> f_derived2(&derived2, &DerivedHandler::on_value_derived);

    AWL_ASSERT_FALSE(f_base == f_other);
    AWL_ASSERT_FALSE(f_base == f_derived1);
    AWL_ASSERT(f_derived1 == f_derived1_dup);
    AWL_ASSERT_FALSE(f_derived1 == f_derived2);

    std::unordered_set<awl::equatable_function<void(int)>> handlers;
    AWL_ASSERT(handlers.insert(f_base).second);
    AWL_ASSERT(handlers.insert(f_other).second);
    AWL_ASSERT(handlers.insert(f_derived1).second);
    AWL_ASSERT_FALSE(handlers.insert(f_derived1_dup).second);
    AWL_ASSERT(handlers.insert(f_derived2).second);
    AWL_ASSERT_EQUAL(4u, handlers.size());

    awl::equatable_function<bool(int)> p_base(&base, &Handler::test_value);
    awl::equatable_function<bool(int)> p_other(&other, &OtherHandler::test_value);
    awl::equatable_function<bool(int)> p_derived(&derived1, &DerivedHandler::test_value_derived);
    awl::equatable_function<bool(int)> p_derived_dup(&derived1, &DerivedHandler::test_value_derived);

    std::unordered_set<awl::equatable_function<bool(int)>> predicates;
    AWL_ASSERT(predicates.insert(p_base).second);
    AWL_ASSERT(predicates.insert(p_other).second);
    AWL_ASSERT(predicates.insert(p_derived).second);
    AWL_ASSERT_FALSE(predicates.insert(p_derived_dup).second);
    AWL_ASSERT_EQUAL(3u, predicates.size());
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
