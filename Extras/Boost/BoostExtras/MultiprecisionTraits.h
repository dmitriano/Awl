/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <boost/multiprecision/cpp_int.hpp>

namespace awl::helpers
{
    template <class T>
    struct multiprecision_descriptor;

    template <>
    struct multiprecision_descriptor<boost::multiprecision::uint128_t>
    {
        using signed_type = boost::multiprecision::int128_t;

        static constexpr std::size_t size = 16;
    };

    template <>
    struct multiprecision_descriptor<boost::multiprecision::uint256_t>
    {
        using signed_type = boost::multiprecision::int256_t;

        static constexpr std::size_t size = 32;
    };

    template <>
    struct multiprecision_descriptor<boost::multiprecision::uint512_t>
    {
        using signed_type = boost::multiprecision::int512_t;

        static constexpr std::size_t size = 64;
    };

    template <>
    struct multiprecision_descriptor<boost::multiprecision::uint1024_t>
    {
        using signed_type = boost::multiprecision::int1024_t;

        static constexpr std::size_t size = 128;
    };

    template <>
    struct multiprecision_descriptor<boost::multiprecision::int128_t>
    {
        using signed_type = boost::multiprecision::int128_t;

        static constexpr std::size_t size = 16;
    };

    template <>
    struct multiprecision_descriptor<boost::multiprecision::int256_t>
    {
        using signed_type = boost::multiprecision::int256_t;

        static constexpr std::size_t size = 32;
    };

    template <>
    struct multiprecision_descriptor<boost::multiprecision::int512_t>
    {
        using signed_type = boost::multiprecision::int512_t;

        static constexpr std::size_t size = 64;
    };

    template <>
    struct multiprecision_descriptor<boost::multiprecision::int1024_t>
    {
        using signed_type = boost::multiprecision::int1024_t;

        static constexpr std::size_t size = 128;
    };

    template <class T>
    using make_signed_t = typename multiprecision_descriptor<T>::signed_type;
}
