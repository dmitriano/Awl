/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/SharedSingleton.h"
#include "Awl/Testing/UnitTest.h"

#include "Tests/Helpers/NonCopyable.h"

#include <memory>

namespace
{
    constexpr int value = 5;

    using A = awl::testing::helpers::NonCopyable;
}

namespace awl
{
    template <>
    std::shared_ptr<A> make_singleton_instance<>()
    {
        return std::make_shared<A>(value);
    }

    template <>
    std::shared_ptr<int> make_singleton_instance<>()
    {
        return std::make_shared<int>(23);
    }
}

AWL_TEST(SharedSingleton)
{
    AWL_UNUSED_CONTEXT;

    AWL_ASSERT_EQUAL(0, A::count);

    {
        auto p1 = awl::shared_singleton<A>();

        AWL_ASSERT_EQUAL(1, A::count);
        AWL_ASSERT(*p1 == A(value));

        {
            auto p2 = awl::shared_singleton<A>();

            AWL_ASSERT_EQUAL(1, A::count);
            AWL_ASSERT(*p2 == A(value));
        }

        AWL_ASSERT_EQUAL(1, A::count);
    }

    AWL_ASSERT_EQUAL(0, A::count);
    AWL_ASSERT(!awl::shared_singleton<A>());
    AWL_ASSERT_EQUAL(0, A::count);
}

AWL_TEST(OnDemandSingleton)
{
    AWL_UNUSED_CONTEXT;

    AWL_ASSERT_EQUAL(0, A::count);

    {
        auto p1 = awl::ondemand_singleton<A>();

        AWL_ASSERT_EQUAL(1, A::count);
        AWL_ASSERT(*p1 == A(value));

        {
            auto p2 = awl::ondemand_singleton<A>();

            AWL_ASSERT_EQUAL(1, A::count);
            AWL_ASSERT(*p2 == A(value));
        }

        AWL_ASSERT_EQUAL(1, A::count);
    }

    AWL_ASSERT_EQUAL(0, A::count);
    AWL_ASSERT(awl::ondemand_singleton<A>() != nullptr);
    AWL_ASSERT_EQUAL(0, A::count);

    {
        auto p1 = awl::ondemand_singleton<A>();

        AWL_ASSERT_EQUAL(1, A::count);
        AWL_ASSERT(*p1 == A(value));

        {
            auto p2 = awl::ondemand_singleton<A>();

            AWL_ASSERT_EQUAL(1, A::count);
            AWL_ASSERT(*p2 == A(value));
        }

        AWL_ASSERT_EQUAL(1, A::count);
    }
}
