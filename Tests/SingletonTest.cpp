/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Tests/Helpers/NonCopyable.h"

#include "Awl/SharedSingleton.h"
#include "Awl/Testing/UnitTest.h"

#include <memory>

namespace
{
    constexpr int shared_value = 5;
    constexpr int weak_value = 7;

    using A = awl::testing::helpers::NonCopyable;
}

namespace awl
{
    template <>
    std::shared_ptr<A> make_shared_instance<>()
    {
        return std::make_shared<A>(shared_value);
    }

    template <>
    std::shared_ptr<A> make_weak_instance<>()
    {
        return std::make_shared<A>(weak_value);
    }

    template <>
    std::shared_ptr<int> make_shared_instance<>()
    {
        return std::make_shared<int>(23);
    }

    template <>
    std::shared_ptr<int> make_weak_instance<>()
    {
        return std::make_shared<int>(29);
    }
}

AWL_TEST(SharedSingleton)
{
    AWL_UNUSED_CONTEXT;

    AWL_ASSERT_EQUAL(23, *awl::shared_singleton<int>());
    AWL_ASSERT_EQUAL(0L, *awl::shared_singleton<long>());

    AWL_ASSERT_EQUAL(0, A::count);

    {
        auto p1 = awl::shared_singleton<A>();

        AWL_ASSERT_EQUAL(1, A::count);
        AWL_ASSERT(*p1 == A(shared_value));

        {
            auto p2 = awl::shared_singleton<A>();

            AWL_ASSERT_EQUAL(1, A::count);
            AWL_ASSERT(*p2 == A(shared_value));
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

    AWL_ASSERT_EQUAL(29, *awl::weak_singleton<int>());
    AWL_ASSERT_EQUAL(0L, *awl::weak_singleton<long>());

    AWL_ASSERT_EQUAL(0, A::count);

    {
        auto p1 = awl::weak_singleton<A>();

        AWL_ASSERT_EQUAL(1, A::count);
        AWL_ASSERT(*p1 == A(weak_value));

        {
            auto p2 = awl::weak_singleton<A>();

            AWL_ASSERT_EQUAL(1, A::count);
            AWL_ASSERT(*p2 == A(weak_value));
        }

        AWL_ASSERT_EQUAL(1, A::count);
    }

    AWL_ASSERT_EQUAL(0, A::count);
    AWL_ASSERT(awl::weak_singleton<A>() != nullptr);
    AWL_ASSERT_EQUAL(0, A::count);

    {
        auto p1 = awl::weak_singleton<A>();

        AWL_ASSERT_EQUAL(1, A::count);
        AWL_ASSERT(*p1 == A(weak_value));

        {
            auto p2 = awl::weak_singleton<A>();

            AWL_ASSERT_EQUAL(1, A::count);
            AWL_ASSERT(*p2 == A(weak_value));
        }

        AWL_ASSERT_EQUAL(1, A::count);
    }
}
