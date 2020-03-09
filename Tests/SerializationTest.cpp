#include "Awl/Serializable.h"
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
    template<std::size_t N>
    struct ct_str
    {
        char state[N + 1] = { 0 };
        constexpr ct_str(char const(&arr)[N + 1])
        {
            for (std::size_t i = 0; i < N; ++i)
                state[i] = arr[i];
        }
        constexpr char operator[](std::size_t i) const { return state[i]; }
        constexpr char& operator[](std::size_t i) { return state[i]; }

        constexpr explicit operator char const*() const { return state; }
        constexpr char const* data() const { return state; }
        constexpr std::size_t size() const { return N; }
        constexpr char const* begin() const { return state; }
        constexpr char const* end() const { return begin() + size(); }

        constexpr ct_str() = default;
        constexpr ct_str(ct_str const&) = default;
        constexpr ct_str& operator=(ct_str const&) = default;

        template<std::size_t M>
        friend constexpr ct_str<N + M> operator+(ct_str lhs, ct_str<M> rhs)
        {
            ct_str<N + M> retval;
            for (std::size_t i = 0; i < N; ++i)
                retval[i] = lhs[i];
            for (std::size_t i = 0; i < M; ++i)
                retval[N + i] = rhs[i];
            return retval;
        }

        friend constexpr bool operator==(ct_str lhs, ct_str rhs)
        {
            for (std::size_t i = 0; i < N; ++i)
                if (lhs[i] != rhs[i]) return false;
            return true;
        }
        friend constexpr bool operator!=(ct_str lhs, ct_str rhs)
        {
            for (std::size_t i = 0; i < N; ++i)
                if (lhs[i] != rhs[i]) return true;
            return false;
        }
        
        template<std::size_t M, std::enable_if_t< M != N, bool > = true>
        friend constexpr bool operator!=(ct_str lhs, ct_str<M> rhs)
        {
            static_cast<void>(lhs);
            static_cast<void>(rhs);
            return true;
        }

        template<std::size_t M, std::enable_if_t< M != N, bool > = true>
        friend bool operator==(ct_str, ct_str<M>)
        {
            static_cast<void>(lhs);
            static_cast<void>(rhs);
            return false;
        }
    };

    template<std::size_t N>
    ct_str(char const(&)[N])->ct_str<N - 1>;

    constexpr ct_str hello = "hello";
    constexpr ct_str world = "world";
    constexpr ct_str hello_world = hello + world;

    class type_info
    {
    public:

        constexpr type_info(const char * name) : m_name(name)
        {
        }

        constexpr const char * name() const noexcept
        {
            return m_name;
        }

        constexpr bool operator == (const type_info & other)
        {
            const char * a = name();
            const char * b = other.name();

            while (*a != 0 && *b != 0)
            {
                if (*a++ != *b++)
                {
                    return false;
                }
            }

            return *a == 0 && *b == 0;
        }

    private:

        const char * m_name;

        friend constexpr const char * make_type_info();
    };
    
    template <class T>
    constexpr auto get_arithmetic_size()
    {
        if constexpr (sizeof(T) == 1)
            return ct_str{ "1" };
        else if constexpr (sizeof(T) == 2)
            return ct_str{ "2" };
        else if constexpr (sizeof(T) == 4)
            return ct_str{ "4" };
        else if constexpr (sizeof(T) == 8)
            return ct_str{ "8" };
        else if constexpr (sizeof(T) == 16)
            return ct_str{ "16" };
        else static_assert(dependent_false_v<T>);
    }

    template <class T, std::enable_if_t<std::is_arithmetic<T>{}, bool> = true>
    constexpr auto make_type_name()
    {
        if constexpr (std::is_signed<T>{})
            return ct_str{ "int" } + get_arithmetic_size<T>();
        else
            return ct_str{ "uint" } + get_arithmetic_size<T>();
    }

    static_assert(make_type_name<int>() == make_type_name<int32_t>());
    static_assert(hello_world != hello);

    template <class T>
    constexpr std::enable_if_t<std::is_arithmetic_v<T>, type_info> make_type_info()
    {
        const char * prefix = std::is_signed_v<T> ? "int" : "uint";
        return type_info(prefix);// +get_arithmetic_size<T>());
    }

    //template <class T, class Allocator = std::allocator<T>>
    //constexpr type_info make_type_info(std::vector<T, Allocator> & v)
    //{
    //    static_cast<void>(val);
    //    return "vector" + make_type_info;
    //};

    static_assert(awl::make_type_info<int>() == awl::make_type_info<long>());

    template<typename Test, template<typename...> class Ref>
    struct is_specialization : std::false_type {};

    template<template<typename...> class Ref, typename... Args>
    struct is_specialization<Ref<Args...>, Ref> : std::true_type {};

    //template<template<typename...> class Ref, typename... Args>
    //constexpr bool is_specialization_v = is_specialization<Ref<Args...>, Ref>::value;

    template <class T>
    constexpr std::enable_if_t<is_specialization<T, std::vector>::value || is_specialization<T, std::list>::value, type_info> make_type_info()
    {
        return type_info("sequence");// + make_type_info<typename T::value_type>;
    }

    static_assert(awl::make_type_info<std::vector<int>>() == awl::make_type_info<std::list<int>>());
}

/*
AWT_TEST(TypeName)
{
    context.out << awl::FromACString(awl::type_info<int>().name()) << std::endl;
}
*/