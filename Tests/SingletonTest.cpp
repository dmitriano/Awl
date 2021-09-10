/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
    bool is_uninitialized(std::weak_ptr<T> const& weak)
    {
        using wt = std::weak_ptr<T>;
        return !weak.owner_before(wt{}) && !wt{}.owner_before(weak);
    }

    template <class T>
    std::shared_ptr<T> CreateObject();

    template <>
    std::shared_ptr<A> CreateObject()
    {
        //We have a long leaving std::weak_ptr so we do not use std::make_shared.
        return std::shared_ptr<A>(new A(value));
    }

    template <class T>
    std::shared_ptr<T> GetObject()
    {
        static std::weak_ptr<T> wp;

        if (is_uninitialized(wp))
        {
            std::shared_ptr<T> p = CreateObject<T>();

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
        auto p1 = GetObject<A>();

        AWT_ASSERT_EQUAL(1, A::count);
        AWT_ASSERT(*p1 == A(value));

        {
            auto p2 = GetObject<A>();

            AWT_ASSERT_EQUAL(1, A::count);
            AWT_ASSERT(*p2 == A(value));
        }

        AWT_ASSERT_EQUAL(1, A::count);
    }

    AWT_ASSERT_EQUAL(0, A::count);
    AWT_ASSERT(!GetObject<A>());
    AWT_ASSERT_EQUAL(0, A::count);
}
