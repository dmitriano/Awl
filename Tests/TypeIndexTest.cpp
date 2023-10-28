/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Tuplizable.h"
#include "Awl/Io/TypeIndex.h"

#include <string>
#include <vector>

using namespace awl::io;

namespace
{
    struct A
    {
        bool x;
        int y;

        AWL_TUPLIZABLE(x, y)
    };

    struct B
    {
        A a;
        double z;

        AWL_TUPLIZABLE(a, z)
    };

    struct C
    {
        std::string d;
        B b;
        A a;
        std::vector<std::string> e;

        AWL_TUPLIZABLE(d, b, a, e)
    };

    struct D
    {
        C c;
        B b;
        A a;

        AWL_TUPLIZABLE(c, b, a)
    };

    static_assert(countof_fields<A>() == 2);
    static_assert(countof_fields<B>() == 3);

    static_assert(indexof_type<A, bool>() == 0);
    static_assert(indexof_type<A, int>() == 1);
    static_assert(indexof_type<B, bool>() == 0);
        
    static_assert(indexof_type<B, bool>() == 0);
    static_assert(indexof_type<B, int>() == 1);
    static_assert(indexof_type<B, double>() == 2);
    static_assert(indexof_type<B, std::string>() == no_type_index);
        
    static_assert(indexof_type<C, bool>() == 0 + 1);
    static_assert(indexof_type<C, int>() == 1 + 1);
    static_assert(indexof_type<C, double>() == 2 + 1);
    static_assert(indexof_type<C, std::string>() == 0);
    static_assert(indexof_type<C, std::vector<std::string>>() == countof_fields<B>() + countof_fields<A>() + 1);
    static_assert(indexof_type<C, std::vector<int>>() == no_type_index);

    static_assert(indexof_type<D, bool>() == 0 + 1);
    static_assert(indexof_type<D, int>() == 1 + 1);
    static_assert(indexof_type<D, double>() == 2 + 1);
    static_assert(indexof_type<D, std::string>() == 0);
    static_assert(indexof_type<D, std::vector<std::string>>() == countof_fields<B>() + countof_fields<A>() + 1);
    static_assert(indexof_type<D, std::vector<int>>() == no_type_index);
}
