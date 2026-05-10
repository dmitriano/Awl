/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Mp/TypeDescriptor.h"

#include <boost/multiprecision/cpp_int.hpp>

namespace awl::mp
{
    template <class T> requires (
        std::is_same_v<T, boost::multiprecision::int128_t> ||
        std::is_same_v<T, boost::multiprecision::uint128_t> ||
        std::is_same_v<T, boost::multiprecision::int256_t> ||
        std::is_same_v<T, boost::multiprecision::uint256_t> ||
        std::is_same_v<T, boost::multiprecision::int512_t> ||
        std::is_same_v<T, boost::multiprecision::uint512_t> ||
        std::is_same_v<T, boost::multiprecision::int1024_t> ||
        std::is_same_v<T, boost::multiprecision::uint1024_t >
    )
    struct type_descriptor<T>
    {
        using inner_tuple = std::tuple<>;

        static constexpr std::string name()
        {
            return "multi_int";
        }
    };
}
