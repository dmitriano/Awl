#include "Awl/Serializable.h"
#include "Awl/FixedString.h"
#include "Awl/Testing/UnitTest.h"
#include "Awl/Io/RwHelpers.h"

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

namespace awl
{
    template <class T>
    constexpr auto get_arithmetic_size()
    {
        if constexpr (sizeof(T) == 1)
            return FixedString{ "8" };
        else if constexpr (sizeof(T) == 2)
            return FixedString{ "16" };
        else if constexpr (sizeof(T) == 4)
            return FixedString{ "32" };
        else if constexpr (sizeof(T) == 8)
            return FixedString{ "64" };
        else if constexpr (sizeof(T) == 16)
            return FixedString{ "128" };
        else static_assert(dependent_false_v<T>);
    }

    template <class T, std::enable_if_t<std::is_arithmetic<T>{}, bool> = true>
    constexpr auto make_type_name()
    {
        if constexpr (std::is_signed<T>{})
        {
            return FixedString{ "int" } +get_arithmetic_size<T>() + FixedString{ "_t" };
        }
        else
        {
            return FixedString{ "uint" } +get_arithmetic_size<T>() + FixedString{ "_t" };
        }
    }

    static_assert(make_type_name<int32_t>() == FixedString{ "int32_t" });
    static_assert(make_type_name<uint16_t>() == FixedString{ "uint16_t" });

    template<typename Test, template<typename...> class Ref>
    struct is_specialization : std::false_type {};

    template<template<typename...> class Ref, typename... Args>
    struct is_specialization<Ref<Args...>, Ref> : std::true_type {};

    //template<template<typename...> class Ref, typename... Args>
    //constexpr bool is_specialization_v = is_specialization<Ref<Args...>, Ref>::value;

    template <class T, std::enable_if_t<is_specialization<T, std::vector>::value || is_specialization<T, std::list>::value, bool> = true>
    constexpr auto make_type_name()
    {
        return FixedString("sequence<") + make_type_name<typename T::value_type>() + FixedString(">");
    }

    static_assert(make_type_name<std::vector<int32_t>>() == FixedString{ "sequence<int32_t>" });
    static_assert(make_type_name<std::vector<std::list<uint64_t>>>() == FixedString{ "sequence<sequence<uint64_t>>" });
}

/*
AWT_TEST(TypeName)
{
    context.out << awl::FromACString(awl::type_info<int>().name()) << std::endl;
}
*/