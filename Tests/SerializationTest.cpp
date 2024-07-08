/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Tuplizable.h"
#include "Awl/FixedString.h"
#include "Awl/Testing/UnitTest.h"
#include "Awl/Io/ReadWrite.h"
#include "Awl/Io/TypeName.h"
#include "Awl/Io/TypeHash.h"
#include "Awl/Io/TypeIndex.h"

#include <string>
#include <vector>
#include <list>

using namespace awl::testing;
using namespace awl::io;

namespace awl
{
    //MSVC 2017: error C2131: expression did not evaluate to a constant.
    /*
    template <class T, typename Func>
    inline constexpr void for_each_member(const T & val, const Func & f)
    {
        static_assert(std::is_arithmetic<T>::value || tuplizable<T>);

        if constexpr (std::is_arithmetic<T>::value)
        {
            f(val);
        }
        else if constexpr (tuplizable<T>)
        {
            for_each(object_as_tuple(val), f);
        }
    }

    template <class T>
    constexpr std::size_t sizeof_object(const T & val)
    {
        std::size_t size = 0;

        for_each_member(val, [&size](auto& field) { size += sizeof_object(field); });

        return size;
    }
    */

    template <class T>
    constexpr std::size_t sizeof_object(const T & val)
    {
        static_assert(std::is_arithmetic<T>::value || tuplizable<T>);

        if constexpr (std::is_arithmetic<T>::value)
        {
            return sizeof(val);
        }
        else if constexpr (tuplizable<T>)
        {
            std::size_t size = 0;

            for_each(object_as_tuple(val), [&size](auto& field) { size += sizeof_object(field); });

            return size;
        }
        else
        {
            static_assert(dependent_false_v<T>);
        }
    }

    template <class T>
    constexpr std::size_t sizeof_class()
    {
        using ConstT = const T;
        
        return sizeof_object(ConstT{});
    }

    template <class T>
    constexpr bool is_object_fixed_size(const T & val)
    {
        if constexpr (std::is_arithmetic<T>::value)
        {
            static_cast<void>(val);
            return true;
        }
        else if constexpr (tuplizable<T>)
        {
            bool all = true;

            for_each(object_as_tuple(val), [&all](auto& field)
            {
                if (!is_object_fixed_size(field))
                {
                    all = false;
                }
            });

            return all;
        }

        static_cast<void>(val);
        return false;
    }

    template <class T>
    constexpr bool is_class_fixed_size()
    {
        using ConstT = const T;

        return is_object_fixed_size(ConstT{});
    }

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
            //prevents B from being constexpr
            //std::string x;
            double z;

            AWL_TUPLIZABLE(a, z)
        };

        struct C
        {
            A a;
            int * p;

            AWL_TUPLIZABLE(a, p)
        };

        static_assert(tuplizable<A>);

        static_assert(sizeof_class<A>() == sizeof(bool) + sizeof(int));
        static_assert(sizeof_class<B>() == sizeof_class<A>() + sizeof(double));

        static_assert(is_class_fixed_size<A>());
        static_assert(is_class_fixed_size<B>());
        static_assert(!is_class_fixed_size<C>());

        static_assert(countof_fields<A>() == 2);
        static_assert(countof_fields<B>() == 3);
   }
}

/*
AWT_TEST(TypeName)
{
    context.out << awl::FromACString(awl::type_info<int>().name()) << std::endl;
}
*/
