/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Stringizable.h"
#include "Awl/Io/MpHelpers.h"

#include <chrono>
#include <vector>
#include <set>

namespace awl::testing::helpers
{
    template <class T>
    using Allocator = std::allocator<T>;

    /*
    thread_local awl::TrivialSpace trivialSpace;
    thread_local awl::TrivialAllocator<void> alloc(trivialSpace);
    thread_local awl::TrivialAllocator<void> expectedAlloc(trivialSpace);
    */

    using String = std::basic_string<char, std::char_traits<char>, Allocator<char>>;
    
    template <class T>
    using Vector = std::vector<T, Allocator<T>>;

    /*
    //It can be something like this in a future version:
    template <template <class> class Allocator>
    struct A1
    {
        A1(Allocator<void> alloc) : c(Allocator<char>(alloc)) {}

        int a;
        double b;
        String c;

        AWL_STRINGIZABLE(a, b, c)
    };
    */

    namespace v1
    {
        //But currently we use default constructible allocator.
        struct A
        {
            int a;
            bool b;
            String c;
            double d;

            AWL_STRINGIZABLE(a, b, c, d)
        };

        AWL_MEMBERWISE_EQUATABLE(A)

        inline const A a_expected = { 1, true, "abc", 2.0 };

        static_assert(std::is_same_v<std::variant<A, int, bool, String, double>, awl::io::helpers::variant_from_struct<A>>);

        struct C
        {
            int x;
            A a;

            AWL_STRINGIZABLE(x, a)
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

            AWL_STRINGIZABLE(a, b, x, y, v, v1)
        };

        AWL_MEMBERWISE_EQUATABLE(B)

        inline const B b_expected = { a_expected, a_expected, 1, true, Vector<A>{ a_expected, a_expected, a_expected }, { c_expected } };
        
        static_assert(std::is_same_v<std::variant<B, A, int, bool, String, double, Vector<A>, std::set<C>>, awl::io::helpers::variant_from_struct<B>>);
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

            AWL_STRINGIZABLE(b, d, e, f, c)
        };

        AWL_MEMBERWISE_EQUATABLE(A)

        inline const A a_expected = { v1::a_expected.b, v1::a_expected.d, 5, "xyz", v1::a_expected.c };

        static_assert(std::is_same_v<std::variant<A, bool, double, int, String>, awl::io::helpers::variant_from_struct<A>>);

        struct C
        {
            int x;
            A a;

            AWL_STRINGIZABLE(x, a)
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

            AWL_STRINGIZABLE(a, x, z, w, v, v1, v2)
        };

        AWL_MEMBERWISE_EQUATABLE(B)

        inline const B b_expected = { v2::a_expected, Vector<int>{ 1, 2, 3 },  v1::b_expected.x, "xyz", Vector<A>{ a_expected, a_expected, a_expected }, { c_expected }, {}};

        static_assert(std::is_same_v<std::variant<B, A, bool, double, int, String, Vector<int>, Vector<A>, std::set<C>, Vector<C>>, awl::io::helpers::variant_from_struct<B>>);
        static_assert(std::is_same_v<std::variant<B, A, bool, double, int, String, Vector<int>, Vector<A>, std::set<C>, Vector<C>, float>, awl::io::helpers::variant_from_structs<B, float>>);

        static_assert(std::is_same_v<std::variant<B, A, bool, double, int, String, Vector<int>, Vector<A>, std::set<C>, Vector<C>, C, float>, awl::io::helpers::variant_from_structs<B, C, float>>);
    }

    //using V1 = std::variant<v1::A, v1::B, bool, char, int, float, double, String>;
    //using V2 = std::variant<v2::A, v2::B, bool, char, int, float, double, String, v2::C, Vector<int>>;

    using V1 = awl::io::helpers::variant_from_structs<v1::B, v1::C>;
    using V2 = awl::io::helpers::variant_from_structs<v2::B, v2::C>;
}
