#include "Awl/Prototype.h"
#include "Awl/Testing/UnitTest.h"

#include <string>
#include <array>
#include <sstream>
#include <any>
#include <map>

using namespace awl::testing;

namespace
{
    struct A
    {
        int x;
        double y;
        std::string z;

        AWL_STRINGIZABLE(x, y, z)
    };

    AWL_MEMBERWISE_EQUATABLE(A)

    struct B
    {
        int x;
        A a1;
        double y;
        A a2;
        std::string z;

        AWL_STRINGIZABLE(x, a1, y, a2, z)
    };

    AWL_MEMBERWISE_EQUATABLE(B)
}

using V = std::variant<bool, char, int, float, double, std::string>;

AWT_TEST(Prototype_TypeMap)
{
    AWT_UNUSED_CONTEXT;

    awl::AttachedPrototype<V, A> ap;

    awl::DetachedPrototype dp(ap);

    awl::DetachedPrototype result(std::vector<awl::DetachedPrototype::FieldContainer>{ {"x", 2u}, { "y", 4u }, { "z", 5u } });

    AWT_ASSERT(dp == result);
}

namespace awl
{
    namespace helpers
    {
        template <class Variant, class Tuple, std::size_t Index = 0>
        Variant runtime_get(Tuple &&tuple, std::size_t index)
        {
            if constexpr (Index == std::tuple_size_v<std::decay_t<Tuple>>)
            {
                static_cast<void>(tuple);
                static_cast<void>(index);
                throw GeneralException(_T("Index out of range for tuple"));
            }
            else
            {
                if (index == Index)
                {
                    return Variant{ std::get<Index>(tuple) };
                }

                return runtime_get<Variant, Tuple, Index + 1>(std::forward<Tuple>(tuple), index);
            }
        }

        template <class Tuple, class Variant, std::size_t Index = 0>
        void runtime_set(Tuple &tuple, std::size_t index, Variant const& variant)
        {
            if constexpr (Index == std::tuple_size_v<std::decay_t<Tuple>>)
            {
                static_cast<void>(tuple);
                static_cast<void>(index);
                static_cast<void>(variant);
                throw GeneralException(_T("Index out of range for tuple"));
            }
            else
            {
                if (index == Index)
                {
                    // Note: You should check here that variant holds the correct type
                    // before assigning.
                    std::get<Index>(tuple) = std::get<std::remove_reference_t<std::tuple_element_t<Index, Tuple>>>(variant);
                }
                else
                {
                    runtime_set<Tuple, Variant, Index + 1>(tuple, index, variant);
                }
            }
        }
    }

    template <class V, class S>
    class Accessor
    {
    private:

        using Tie = typename tuplizable_traits<S>::Tie;

    public:

        V Get(const S & val, size_t index) const
        {
            return helpers::runtime_get<V>(object_as_tuple(val), index);
        }

        void Set(S & val, size_t index, V v_field) const
        {
            auto temp = val.as_tuple();
            helpers::runtime_set(temp, index, v_field);
        }
    };
}

AWT_TEST(Prototype_RuntimeIndex)
{
    AWT_UNUSED_CONTEXT;

    using Variant = std::variant<bool, char, int, float, double, std::string>;
    using Tuple = std::tuple<char, int, int, double, std::string>;

    using namespace std::string_literals;
    Tuple t = std::make_tuple('a', 2, 3, 5.0, "abc");
    Variant v = awl::helpers::runtime_get<Variant>(t, 1);
    AWT_ASSERT(std::get<int>(v) == 2);
    awl::helpers::runtime_set(t, 4, Variant("xyz"s));
    AWT_ASSERT(std::get<4>(t) == std::string("xyz"));
}

AWT_TEST(Prototype_GetSetPlain)
{
    AWT_UNUSED_CONTEXT;

    awl::Accessor<V, A> ap;

    A a = { 1, 5.0, "abc" };

    AWT_ASSERT(ap.Get(a, 0) == V(a.x));
    AWT_ASSERT(ap.Get(a, 1) == V(a.y));
    AWT_ASSERT(ap.Get(a, 2) == V(a.z));

    ap.Set(a, 0, 3);
    ap.Set(a, 1, 7.0);
    ap.Set(a, 2, std::string("xyz"));

    AWT_ASSERT(ap.Get(a, 0) == V(3));
    AWT_ASSERT(ap.Get(a, 1) == V(7.0));
    AWT_ASSERT(ap.Get(a, 2) == V(std::string("xyz")));
}

AWT_TEST(Prototype_GetSetRecursive)
{
    AWT_UNUSED_CONTEXT;

    //awl::Accessor<V, B> bp;

    A a = { 1, 5.0, "abc" };
    B b = { 1, a, 5.0, a, "abc"};

    /*
    AWT_ASSERT(ap.Get(a, 0) == V(a.x));
    AWT_ASSERT(ap.Get(a, 1) == V(a.y));
    AWT_ASSERT(ap.Get(a, 2) == V(a.z));

    ap.Set(a, 0, 3);
    ap.Set(a, 1, 7.0);
    ap.Set(a, 2, std::string("xyz"));

    AWT_ASSERT(ap.Get(a, 0) == V(3));
    AWT_ASSERT(ap.Get(a, 1) == V(7.0));
    AWT_ASSERT(ap.Get(a, 2) == V(std::string("xyz")));
    */
}
