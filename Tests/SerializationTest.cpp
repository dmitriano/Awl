#include "Awl/Serializable.h"
#include "Awl/Testing/UnitTest.h"
#include "Awl/Io/RwHelpers.h"

#include <string>

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
    constexpr const char * get_arithmetic_size()
    {
        switch (sizeof(T))
        {
        case 1: return "1";
        case 2: return "2";
        case 4: return "4";
        case 8: return "8";
        case 16: return "16";
        default: static_assert(dependent_false_v<T>);
        }
    }
    
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
}

/*
AWT_TEST(TypeName)
{
    context.out << awl::FromACString(awl::type_info<int>().name()) << std::endl;
}
*/