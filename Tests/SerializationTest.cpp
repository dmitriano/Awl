#include "Awl/Serializable.h"
#include "Awl/Testing/UnitTest.h"

using namespace awl::testing;

namespace awl
{
    template <class T>
    constexpr std::size_t sizeof_object(const T & val)
    {
        if constexpr (std::is_arithmetic<T>::value)
        {
            return sizeof(val);
        }

        if constexpr (std::is_class<T>::value)
        {
            std::size_t size = 0;

            for_each(object_as_tuple(val), [&size](auto& field) { size += sizeof_object(field); });

            return size;
        }

        return 0;
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

        using ConstB = const B;
        
        static_assert(sizeof_object(ConstB{}) == sizeof(bool) + sizeof(int) + sizeof(double));
    }
}
