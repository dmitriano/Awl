#include "Awl/Serializable.h"
#include "Awl/FixedString.h"
#include "Awl/Testing/UnitTest.h"
#include "Awl/Io/RwHelpers.h"
#include "Awl/Io/TypeName.h"

#include <string>
#include <vector>
#include <list>
#include <type_traits>

using namespace awl::testing;

namespace awl
{
    //MSVC 2017: error C2131: expression did not evaluate to a constant.
    /*
    template <class T, typename Func>
    inline constexpr void for_each_member(const T & val, const Func & f)
    {
        static_assert(std::is_arithmetic<T>::value || is_tuplizable_v<T>);

        if constexpr (std::is_arithmetic<T>::value)
        {
            f(val);
        }
        else if constexpr (is_tuplizable_v<T>)
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
        static_assert(std::is_arithmetic<T>::value || is_tuplizable_v<T>);

        if constexpr (std::is_arithmetic<T>::value)
        {
            return sizeof(val);
        }
        else if constexpr (is_tuplizable_v<T>)
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
        else if constexpr (is_tuplizable_v<T>)
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

    namespace static_test
    {
        struct A
        {
            bool x;
            int y;

            AWL_SERIALIZABLE(x, y)
        };

        struct B
        {
            A a;
            //prevents B from being constexpr
            //std::string x;
            double z;

            AWL_SERIALIZABLE(a, z)
        };

        struct C
        {
            B b;
            //std::string s;
            int * p;

            AWL_SERIALIZABLE(b, p)
        };

        static_assert(is_tuplizable_v<A>);

        static_assert(sizeof_class<A>() == sizeof(bool) + sizeof(int));
        static_assert(sizeof_class<B>() == sizeof_class<A>() + sizeof(double));

        static_assert(is_class_fixed_size<A>());
        static_assert(is_class_fixed_size<B>());
        static_assert(!is_class_fixed_size<C>());
    }
}

/*
AWT_TEST(TypeName)
{
    context.out << awl::FromACString(awl::type_info<int>().name()) << std::endl;
}
*/