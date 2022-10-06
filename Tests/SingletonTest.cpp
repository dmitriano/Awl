/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Experimental/Destructible.h"
#include "Awl/ScopeGuard.h"
#include "Awl/SharedSingleton.h"
#include "Awl/Testing/UnitTest.h"

#include "Helpers/NonCopyable.h"

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

AWT_TEST(Destructible)
{
    AWT_UNUSED_CONTEXT;
    
    AWT_ASSERT_EQUAL(0, A::count);

    {
        awl::Destructible<A> da(value);

        auto guard = awl::make_scope_guard([&da]() { da.Destroy();  });

        AWT_ASSERT(*da == A(value));
    }

    AWT_ASSERT_EQUAL(0, A::count);
}

AWT_TEST(SharedSingleton)
{
    AWT_UNUSED_CONTEXT;

    AWT_ASSERT_EQUAL(0, A::count);

    {
        auto p1 = awl::shared_singleton<A>();

        AWT_ASSERT_EQUAL(1, A::count);
        AWT_ASSERT(*p1 == A(value));

        {
            auto p2 = awl::shared_singleton<A>();

            AWT_ASSERT_EQUAL(1, A::count);
            AWT_ASSERT(*p2 == A(value));
        }

        AWT_ASSERT_EQUAL(1, A::count);
    }

    AWT_ASSERT_EQUAL(0, A::count);
    AWT_ASSERT(!awl::shared_singleton<A>());
    AWT_ASSERT_EQUAL(0, A::count);
}

AWT_TEST(OnDemandSingleton)
{
    AWT_UNUSED_CONTEXT;

    AWT_ASSERT_EQUAL(0, A::count);

    {
        auto p1 = awl::ondemand_singleton<A>();

        AWT_ASSERT_EQUAL(1, A::count);
        AWT_ASSERT(*p1 == A(value));

        {
            auto p2 = awl::ondemand_singleton<A>();

            AWT_ASSERT_EQUAL(1, A::count);
            AWT_ASSERT(*p2 == A(value));
        }

        AWT_ASSERT_EQUAL(1, A::count);
    }

    AWT_ASSERT_EQUAL(0, A::count);
    AWT_ASSERT(awl::ondemand_singleton<A>() != nullptr);
    AWT_ASSERT_EQUAL(0, A::count);

    {
        auto p1 = awl::ondemand_singleton<A>();

        AWT_ASSERT_EQUAL(1, A::count);
        AWT_ASSERT(*p1 == A(value));

        {
            auto p2 = awl::ondemand_singleton<A>();

            AWT_ASSERT_EQUAL(1, A::count);
            AWT_ASSERT(*p2 == A(value));
        }

        AWT_ASSERT_EQUAL(1, A::count);
    }
}
