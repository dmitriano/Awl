/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Tests/Helpers/NonCopyableInt.h"

#include "Awl/NoCopyMove.h"
#include "Awl/SharedSingleton.h"
#include "Awl/Testing/UnitTest.h"

#include <memory>

namespace
{
    constexpr int shared_value = 5;
    constexpr int weak_value = 7;

    class A : public awl::testing::helpers::NonCopyableInt, private awl::NoCopyMove
    {
    private:

        friend std::shared_ptr<A> awl::make_shared_instance<A>();

        // Instances can only be created as shared singletons.
        explicit A(const int value) : NonCopyableInt(value)
        {}
    };

    class B : public awl::testing::helpers::NonCopyableInt, private awl::NoCopyMove
    {
    private:

        friend std::shared_ptr<B> awl::make_weak_instance<B>();

        // Instances can only be created as weak singletons.
        explicit B(const int value) : NonCopyableInt(value)
        {}
    };
}

namespace awl
{
    template <>
    std::shared_ptr<A> make_shared_instance<>()
    {
        return std::shared_ptr<A>(new A(shared_value));
    }

    template <>
    std::shared_ptr<B> make_weak_instance<>()
    {
        return std::shared_ptr<B>(new B(weak_value));
    }
}

AWL_TEST(SharedSingleton)
{
    AWL_UNUSED_CONTEXT;

    AWL_ASSERT_EQUAL(0, A::count);

    {
        std::shared_ptr<A> p1 = awl::shared_singleton<A>();

        AWL_ASSERT_EQUAL(1, A::count);
        AWL_ASSERT(*p1 == shared_value);

        {
            std::shared_ptr<A> p2 = awl::shared_singleton<A>();

            AWL_ASSERT_EQUAL(1, A::count);
            AWL_ASSERT(*p2 == shared_value);
        }

        AWL_ASSERT_EQUAL(1, A::count);
    }

    AWL_ASSERT_EQUAL(0, A::count);
    AWL_ASSERT(!awl::shared_singleton<A>());
    AWL_ASSERT_EQUAL(0, A::count);
}

AWL_TEST(WeakSingleton)
{
    AWL_UNUSED_CONTEXT;

    AWL_ASSERT_EQUAL(0, B::count);

    {
        std::shared_ptr<B> p1 = awl::weak_singleton<B>();

        AWL_ASSERT_EQUAL(1, B::count);
        AWL_ASSERT(*p1 == weak_value);

        {
            std::shared_ptr<B> p2 = awl::weak_singleton<B>();

            AWL_ASSERT_EQUAL(1, B::count);
            AWL_ASSERT(*p2 == weak_value);
        }

        AWL_ASSERT_EQUAL(1, B::count);
    }

    AWL_ASSERT_EQUAL(0, B::count);
    AWL_ASSERT(awl::weak_singleton<B>() != nullptr);
    AWL_ASSERT_EQUAL(0, B::count);

    {
        std::shared_ptr<B> p1 = awl::weak_singleton<B>();

        AWL_ASSERT_EQUAL(1, B::count);
        AWL_ASSERT(*p1 == weak_value);

        {
            std::shared_ptr<B> p2 = awl::weak_singleton<B>();

            AWL_ASSERT_EQUAL(1, B::count);
            AWL_ASSERT(*p2 == weak_value);
        }

        AWL_ASSERT_EQUAL(1, B::count);
    }
}
