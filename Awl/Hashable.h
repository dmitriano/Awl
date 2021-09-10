/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Serializable.h"

#include <functional>

namespace awl
{
    template <class T>
    void CombineHash(std::size_t& seed, T const& v)
    {
        seed ^= std::hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    template <class Tuple, size_t Index = std::tuple_size<Tuple>::value - 1>
    struct HashValueImpl
    {
        static void apply(size_t& seed, Tuple const& tuple)
        {
            HashValueImpl<Tuple, Index - 1>::apply(seed, tuple);
            CombineHash(seed, std::get<Index>(tuple));
        }
    };

    template <class Tuple>
    struct HashValueImpl<Tuple, 0>
    {
        static void apply(size_t& seed, Tuple const& tuple)
        {
            CombineHash(seed, std::get<0>(tuple));
        }
    };

    template <typename... Ts>
    size_t GetTupleHash(std::tuple<Ts...> const& t)
    {
        size_t seed = 0;
        HashValueImpl<std::tuple<Ts...>>::apply(seed, t);
        return seed;
    }
}

namespace std
{
    template<typename T>
    struct hash<vector<T>>
    {
        typedef vector<T> argument_type;
        typedef std::size_t result_type;
        result_type operator()(argument_type const& in) const
        {
            size_t seed = 0;

            for (const T & val : in)
            {
                //Combine the hash of the current vector with the hashes of the previous ones
                awl::CombineHash(seed, val);
            }

            return seed;
        }
    };
}

#define AWL_HASHABLE(ClassName) \
    namespace std \
    { \
        template <> \
        struct hash<ClassName> \
        { \
            size_t operator()(const ClassName& val) const \
            { \
                return awl::GetTupleHash(awl::object_as_tuple(val)); \
            } \
        }; \
    }
