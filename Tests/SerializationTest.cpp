#include "Awl/Serializable.h"
#include "Awl/Testing/UnitTest.h"

using namespace awl::testing;

namespace awl
{
    template <typename T, typename = void>
    struct is_tuplizable_t : std::false_type {};

    template <typename T>
    struct is_tuplizable_t <T, std::void_t<decltype(T{}.as_const_tuple()) >> : std::true_type {};
    //There can be 
    //static inline constexpr bool isTuplizable = true;
    //in AWL_SERIALIZABLE macro.

    template <typename T>
    inline constexpr bool is_tuplizable = std::is_class<T>::value && is_tuplizable_t<T>::value;

    template <class T>
    constexpr std::size_t sizeof_object(const T & val)
    {
        if constexpr (std::is_arithmetic<T>::value)
        {
            return sizeof(val);
        }
        else if constexpr (is_tuplizable<T>)
        {
            std::size_t size = 0;

            for_each(object_as_tuple(val), [&size](auto& field) { size += sizeof_object(field); });

            return size;
        }
        else
        {
            static_assert(false);
        }
    }

    template <class T>
    constexpr std::size_t sizeof_class()
    {
        using ConstT = const T;
        
        return sizeof_object(ConstT{});
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
            double z;

            AWL_SERIALIZABLE(a, z)
        };

        static_assert(is_tuplizable<A>);

        static_assert(sizeof_class<A>() == sizeof(bool) + sizeof(int));
        static_assert(sizeof_class<B>() == sizeof_class<A>() + sizeof(double));
    }
}
