#include "Awl/ObjectPool.h"
#include "Awl/Testing/UnitTest.h"

using namespace awl::testing;

namespace
{
    //Test if it compiles with multiple links.
    class another_link : public awl::quick_link
    {
    public:

        using quick_link::quick_link;
    };

    class A : public awl::pooled_object, another_link
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
            std::shared_ptr<A> p = pool.make();
        }

        AWT_ASSERT_EQUAL(1, A::elementCount);

        {
            std::shared_ptr<A> p = pool.make();
        }

        AWT_ASSERT_EQUAL(1, A::elementCount);

        {
            std::shared_ptr<A> p1 = pool.make();
            std::shared_ptr<A> p2 = pool.make();
        }

        AWT_ASSERT_EQUAL(2, A::elementCount);

        {
            std::shared_ptr<A> p = pool.make();
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

namespace
{
    class B : public awl::pooled_object, public std::enable_shared_from_this<B>
    {
    public:

        B() : Value(0)
        {
            ++elementCount;
        }

        ~B()
        {
            --elementCount;
        }

        int Value = 0;

        static int elementCount;
    };

    int B::elementCount = 0;
}

AWT_TEST(ObjectPoolSharedFromThis)
{
    AWT_UNUSED_CONTEXT;

    {
        awl::object_pool<B> pool;

        B* raw_p = new B;

        {
            std::shared_ptr<B> p1 = pool.add(raw_p);

            AWT_ASSERT_EQUAL(1, B::elementCount);

            std::shared_ptr<B> p2 = raw_p->shared_from_this();

            AWT_ASSERT(p1 == p2);
        }

        AWT_ASSERT_EQUAL(1, B::elementCount);

        {
            std::shared_ptr<B> p1 = pool.make();

            B* p_instance = p1.get();

            AWT_ASSERT_EQUAL(1, B::elementCount);

            AWT_ASSERT_EQUAL(raw_p, p1.get());

            std::shared_ptr<B> p2 = p_instance->shared_from_this();

            AWT_ASSERT(p1 == p2);
        }

        AWT_ASSERT_EQUAL(1, B::elementCount);
    }

    AWT_ASSERT_EQUAL(0, B::elementCount);
}