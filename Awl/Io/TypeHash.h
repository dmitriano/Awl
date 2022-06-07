/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Crypto/IntHash.h"
#include "Awl/Io/TypeName.h"

namespace awl::io
{
    template <size_t N>
    constexpr uint64_t calc_type_hash(const fixed_string<char, N> & name)
    {
        awl::crypto::Int64Hash hash;
        return hash(name.begin(), name.end());
    }

    template <class T>
    constexpr uint64_t make_type_hash()
    {
        auto name = make_type_name<T>();
        return calc_type_hash(name);
    }

    static_assert(make_type_hash<uint8_t>() != make_type_hash<int16_t>());
}
