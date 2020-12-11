#include "Experimental/Destructible.h"
#include "Awl/ScopeGuard.h"
#include "Awl/Testing/UnitTest.h"

#include "Helpers/NonCopyable.h"

#include <memory>

namespace
{
    constexpr int value = 5;
    
    using A = awl::testing::helpers::NonCopyable;

    template <typename T>
    inline bool is_uninitialized(std::weak_ptr<T> const& weak)
    {
        using wt = std::weak_ptr<T>;
        return !weak.owner_before(wt{}) && !wt{}.owner_before(weak);
    }

    std::shared_ptr<A> GetObject()
    {
        static std::weak_ptr<A> wp;

        if (is_uninitialized(wp))
        {
            //We have a long leaving std::weak_ptr so we do not use std::make_shared.
            std::shared_ptr<A> p(new A(value));

            wp = p;

            return p;
        }

        return wp.lock();
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
        auto p1 = GetObject();

        AWT_ASSERT_EQUAL(1, A::count);
        AWT_ASSERT(*p1 == A(value));

        {
            auto p2 = GetObject();

            AWT_ASSERT_EQUAL(1, A::count);
            AWT_ASSERT(*p2 == A(value));
        }

        AWT_ASSERT_EQUAL(1, A::count);
    }

    AWT_ASSERT_EQUAL(0, A::count);
    AWT_ASSERT(!GetObject());
    AWT_ASSERT_EQUAL(0, A::count);
}
