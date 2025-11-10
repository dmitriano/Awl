/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Experimental/Immutable.h"

#include "Awl/Testing/UnitTest.h"
#include "Awl/StringFormat.h"
#include "Awl/IntRange.h"

#include <vector>
#include <memory>

namespace
{
    struct A
    {
        int x;
        std::string y;

        bool operator ==(const A& other) const = default;
    };

    constexpr awl::immutable<A> useWith()
    {
        awl::immutable<A> a1 = awl::make_immutable<A>(5, "abc");

        awl::immutable<A> a2 = a1.with(&A::x, 10).with(&A::y, std::string("def"));

        return a2;
    }

    // it is entirely constexpr
    static_assert(useWith() == awl::make_immutable<A>(10, "def"));

    struct UniqueA : A
    {
        UniqueA(int x, std::string y) : 
            A(std::move(x), std::move(y)),
            p(std::make_unique<int>(x))
        {}

        bool operator ==(const UniqueA& other) const
        {
            return *(static_cast<const A*>(this)) == static_cast<const A&>(other);
        }

        std::unique_ptr<int> p;
    };

    // We std::unique_ptr with AtomicA, but not immutable.
    struct AtomicA : A
    {
        AtomicA(int x, std::string y) : A(std::move(x), std::move(y)) {}

        bool operator ==(const AtomicA& other) const
        {
            std::lock_guard guard(m);

            return *(static_cast<const A*>(this)) == static_cast<const A&>(other);
        }

        // std::mutex cannot be moved or copied
        mutable std::mutex m;
    };

    template <class A>
    class Test
    {
    public:

        explicit Test(const awl::testing::TestContext& context) : context(context) {}

        void ImmutableConstructorAndOperators()
        {
            awl::immutable<A> a1 = awl::make_immutable<A>(5, "abc");

            // Compiler error:
            // a2->x = 10;

            context.logger.debug(awl::format() << a1->x << " " << a1->y);

            // Copy constructor.
            awl::immutable<A> a2 = a1;

            context.logger.debug(awl::format() << a2->x << " " << a2->y);

            AWL_ASSERT(a2 == a1);

            // Move constructor
            awl::immutable<A> a3 = std::move(a1);

            A mutable_a = a3.release();

            mutable_a.x = 10;

            // Make it immutable again
            awl::immutable<A> a4 = awl::make_immutable<A>(std::move(mutable_a));
        }

        void ImmutableVector()
        {
            AWL_ATTRIBUTE(size_t, insert_count, 1000);

            std::vector<awl::immutable<A>> v;

            const std::string long_string = "A very long string that is not copied, but moved.";

            for (size_t i = 0; i < insert_count; ++i)
            {
                std::ostringstream out;

                out << long_string << " " << i;

                v.push_back(awl::make_immutable<A>(static_cast<int>(i), out.str()));
            }
        }

        void ImmutablePointer()
        {
            // This is possible, but does not make a sense.
            {
                std::unique_ptr<awl::immutable<A>> p = std::make_unique<awl::immutable<A>>(awl::make_immutable<A>(5, "abc"));

                [[maybe_unused]] int x = (*p)->x;
            }

            // To create a smart pointer from awl::immutable we use release method.
            {
                awl::immutable<A> a = awl::make_immutable<A>(5, "abc");

                std::unique_ptr<A> p = std::make_unique<A>(a.release());

                [[maybe_unused]] int x1 = p->x;

                std::unique_ptr<const A> const_p = std::make_unique<const A>(a.release());

                [[maybe_unused]] int x2 = const_p->x;
            }
        }

    private:

        const awl::testing::TestContext& context;
    };
}

AWL_TEST(ImmutableConstructorAndOperators)
{
    // ImmutableConstructorAndOperators test requires A to be copyable.
    Test<A>{context}.ImmutableConstructorAndOperators();
}

AWL_TEST(ImmutableVector)
{
    Test<A>{context}.ImmutableVector();
    Test<UniqueA>{context}.ImmutableVector();
}

AWL_TEST(ImmutablePointer)
{
    Test<A>{context}.ImmutablePointer();
    Test<UniqueA>{context}.ImmutablePointer();
}

#ifdef AWL_DEBUG_IMMUTABLE

namespace
{
    void func(const awl::testing::TestContext& context, awl::immutable<A> a)
    {
        context.logger.debug(awl::format() << a->x << " " << a->y);
    }
}

AWL_TEST(ImmutableException)
{
    awl::immutable<A> a = awl::make_immutable<A>(5, "abc");

    func(context, std::move(a));

    bool thrown = false;

    try
    {
        int x = a->x;

        context.logger.debug(awl::format() << a->x);
    }
    catch (const std::exception& e)
    {
        thrown = true;

        context.logger.debug(e.what());
    }

    AWL_ASSERT(thrown);
}

#endif

// The code demonstrating why do we need immutable.
namespace
{
    // B becomes non-copyable and non-movable.
    struct B
    {
        B() : p(std::make_unique<int>(5)) {}

        const std::unique_ptr<int> p;
    };

    B f1()
    {
        // RVO, so there is no compiler error.
        return {};
    }

    B f2(bool flag)
    {
        // Different variables -> NRVO impossible.
        B b1;
        B b2;

        if (flag)
        {
            // error C2280: '`anonymous-namespace'::B::B(const `anonymous-namespace'::B &)': attempting to reference a deleted function
            // return b1;
        }

        // The same error.
        // return b2;
    }

    B f3()
    {
        // error C2672: 'std::construct_at': no matching overloaded function found
        // std::vector<B> v;
        // v.push_back(B{});
    }
}
