/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Tests/Helpers/NonCopyable.h"

#include "Awl/SharedSingleton.h"
#include "Awl/Testing/UnitTest.h"

#include <memory>
#include <type_traits>

namespace
{
    constexpr int shared_value = 5;
    constexpr int weak_value = 7;

    class A : public awl::testing::helpers::NonCopyable
    {
    private:

        template <class T>
        friend std::shared_ptr<T> awl::make_shared_instance();

        // Instances can only be created as shared singletons.
        A() : NonCopyable(shared_value)
        {}
    };

    class B : public awl::testing::helpers::NonCopyable
    {
    private:

        template <class T>
        friend std::shared_ptr<T> awl::make_weak_instance();

        // Instances can only be created as weak singletons.
        B() : NonCopyable(weak_value)
        {}
    };

    static_assert(!std::is_default_constructible_v<A>);
    static_assert(!std::is_default_constructible_v<B>);
}

namespace awl
{
    template <>
    std::shared_ptr<A> make_shared_instance<>()
    {
        return std::shared_ptr<A>(new A());
    }

    template <>
    std::shared_ptr<B> make_weak_instance<>()
    {
        return std::shared_ptr<B>(new B());
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
