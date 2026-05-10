/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Reflection.h"
#include "Awl/Mp/Mp.h"

#include <chrono>
#include <vector>
#include <set>
#include <map>
#include <string>

namespace awl::testing::helpers
{
    template <class T>
    using Allocator = std::allocator<T>;

    using String = std::basic_string<char, std::char_traits<char>, Allocator<char>>;
    
    template <class T>
    using Vector = std::vector<T, Allocator<T>>;

    namespace v1
    {
        //But currently we use default constructible allocator.
        struct A
        {
            int a;
            bool b;
            String c;
            double d;

            AWL_REFLECT(a, b, c, d)
        };

        AWL_MEMBERWISE_EQUATABLE(A)

        inline const A a_expected = { 1, true, "abc", 2.0 };

        static_assert(std::is_same_v<std::variant<A, int, bool, String, double>, awl::mp::variant_from_struct<A>>);

        struct C
        {
            int x;
            A a;

            AWL_REFLECT(x, a)
        };

        AWL_MEMBERWISE_EQUATABLE(C)

        inline bool operator < (const C & left, const C & right)
        {
            return left.x < right.x;
        }
            
        inline const C c_expected = { 7, a_expected };

        struct B
        {
            A a;
            A b;
            int x;
            bool y;
            Vector<A> v;
            std::set<C> v1;

            AWL_REFLECT(a, b, x, y, v, v1)
        };

        AWL_MEMBERWISE_EQUATABLE(B)

        inline const B b_expected = { a_expected, a_expected, 1, true, Vector<A>{ a_expected, a_expected, a_expected }, { c_expected } };
        
        static_assert(std::is_same_v<std::variant<B, A, int, bool, String, double, Vector<A>, std::set<C>, C>, awl::mp::variant_from_struct<B>>);
    }

    namespace v2
    {
        struct A
        {
            bool b;
            double d;
            int e = 5;
            String f = "xyz";
            String c;

            AWL_REFLECT(b, d, e, f, c)
        };

        AWL_MEMBERWISE_EQUATABLE(A)

        inline const A a_expected = { v1::a_expected.b, v1::a_expected.d, 5, "xyz", v1::a_expected.c };

        static_assert(std::is_same_v<std::variant<A, bool, double, int, String>, awl::mp::variant_from_struct<A>>);

        struct C
        {
            int x;
            A a;

            AWL_REFLECT(x, a)
        };

        AWL_MEMBERWISE_EQUATABLE(C)

        inline bool operator < (const C & left, const C & right)
        {
            return left.x < right.x;
        }

        inline const C c_expected = { 7, a_expected };

        struct B
        {
            A a;
            Vector<int> z{ 1, 2, 3 };
            int x;
            String w = "xyz";
            Vector<A> v;
            std::set<C> v1;
            Vector<C> v2;

            AWL_REFLECT(a, x, z, w, v, v1, v2)
        };

        AWL_MEMBERWISE_EQUATABLE(B)

        inline const B b_expected = { v2::a_expected, Vector<int>{ 1, 2, 3 },  v1::b_expected.x, "xyz", Vector<A>{ a_expected, a_expected, a_expected }, { c_expected }, {}};

        static_assert(std::is_same_v<std::variant<B, A, bool, double, int, String, Vector<int>, Vector<A>, std::set<C>, C, Vector<C>>, awl::mp::variant_from_struct<B>>);
        static_assert(std::is_same_v<std::variant<B, A, bool, double, int, String, Vector<int>, Vector<A>, std::set<C>, C, Vector<C>, float>, awl::mp::variant_from_structs<B, float>>);

        static_assert(std::is_same_v<std::variant<B, A, bool, double, int, String, Vector<int>, Vector<A>, std::set<C>, C, Vector<C>, float>, awl::mp::variant_from_structs<B, C, float>>);
    }

    using V1 = awl::mp::variant_from_structs<v1::B>;
    using V2 = awl::mp::variant_from_structs<v2::B>;
}
