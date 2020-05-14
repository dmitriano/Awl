#pragma once

#include "Awl/Serializable.h"

#include <functional>

namespace awl
{
    template <class T>
    inline void CombineHash(std::size_t& seed, T const& v)
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

    template <typename... TT>
    size_t GetTupleHash(std::tuple<TT...> const& tt)
    {
        size_t seed = 0;
        HashValueImpl<std::tuple<TT...>>::apply(seed, tt);
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
            size_t size = in.size();
            size_t seed = 0;
            for (size_t i = 0; i < size; i++)
                //Combine the hash of the current vector with the hashes of the previous ones
                awl::CombineHash(seed, in[i]);
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
