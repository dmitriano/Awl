#include "Awl/ObjectPool.h"
#include "Awl/Testing/UnitTest.h"

using namespace awl::testing;

namespace
{
    class A : public awl::pooled_object
    {
    public:

        A() : Value(0)
        {
            ++elementCount;
        }

        ~A()
        {
            //awl::quick_link::safe_exclude();

            --elementCount;
        }

        int Value = 0;

        static int elementCount;
    };

    int A::elementCount = 0;
}

AWT_TEST(ObjectPool)
{
    AWT_UNUSED_CONTEXT;

    {
        awl::object_pool<A> pool;

        {
            std::shared_ptr<A> p = pool.make_pooled();
        }

        AWT_ASSERT_EQUAL(1, A::elementCount);

        {
            std::shared_ptr<A> p = pool.make_pooled();
        }

        AWT_ASSERT_EQUAL(1, A::elementCount);

        {
            std::shared_ptr<A> p1 = pool.make_pooled();
            std::shared_ptr<A> p2 = pool.make_pooled();
        }

        AWT_ASSERT_EQUAL(2, A::elementCount);

        {
            std::shared_ptr<A> p = pool.make_pooled();
            std::weak_ptr<A> w = p;
            AWT_ASSERT(w.lock() != nullptr);
            p = nullptr;
            AWT_ASSERT(!w.lock());
        }

        AWT_ASSERT_EQUAL(2, A::elementCount);
    }

    AWT_ASSERT_EQUAL(0, A::elementCount);
}

AWT_TEST(ObjectPoolSingleton)
{
    AWT_UNUSED_CONTEXT;

    {
        {
            std::shared_ptr<A> p = awl::make_pooled<A>();
        }

        AWT_ASSERT_EQUAL(1, A::elementCount);

        {
            std::shared_ptr<A> p = awl::make_pooled<A>();
        }

        AWT_ASSERT_EQUAL(1, A::elementCount);

        {
            std::shared_ptr<A> p1 = awl::make_pooled<A>();
            std::shared_ptr<A> p2 = awl::make_pooled<A>();
        }

        AWT_ASSERT_EQUAL(2, A::elementCount);

        {
            std::shared_ptr<A> p = awl::make_pooled<A>();
            std::weak_ptr<A> w = p;
            AWT_ASSERT(w.lock() != nullptr);
            p = nullptr;
            AWT_ASSERT(!w.lock());
        }

        AWT_ASSERT_EQUAL(2, A::elementCount);
    }

    AWT_ASSERT_EQUAL(2, A::elementCount);

    awl::clear_pool<A>();

    AWT_ASSERT_EQUAL(0, A::elementCount);
}
