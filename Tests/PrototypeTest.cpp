#include "Awl/Io/Prototype.h"
#include "Awl/Testing/UnitTest.h"

#include <string>
#include <vector>
#include <list>
#include <type_traits>

using namespace awl::testing;

namespace
{
    struct A
    {
        int a;
        bool b;
        std::string c;
        double d;

        AWL_STRINGIZABLE(a, b, c, d)
    };

    AWL_MEMBERWISE_EQUATABLE(A)

    struct B
    {
        A a;
        A b;
        int x;
        bool y;

        AWL_STRINGIZABLE(a, b, x, y)
    };

    AWL_MEMBERWISE_EQUATABLE(B)
}

AWT_TEST(Prototype)
{
    awl::io::AttachedPrototype<A> ap;

    AWT_ASSERT(ap.GetCount() == std::tuple_size_v<awl::tuplizable_traits<A>::Tie>);
}
