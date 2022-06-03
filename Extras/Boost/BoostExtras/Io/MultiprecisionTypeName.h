/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/TypeName.h"

#include <boost/multiprecision/cpp_int.hpp>

namespace awl::io
{
    template <class T>
    struct type_descriptor<T, std::enable_if_t<
        std::is_same_v<T, boost::multiprecision::int128_t> ||
        std::is_same_v<T, boost::multiprecision::uint128_t> ||
        std::is_same_v<T, boost::multiprecision::int256_t> ||
        std::is_same_v<T, boost::multiprecision::uint256_t> ||
        std::is_same_v<T, boost::multiprecision::int512_t> ||
        std::is_same_v<T, boost::multiprecision::uint512_t> ||
        std::is_same_v<T, boost::multiprecision::int1024_t> ||
        std::is_same_v<T, boost::multiprecision::uint1024_t>>>
    {
        static constexpr auto name()
        {
            return fixed_string("multi_int");
        }
    };
}
