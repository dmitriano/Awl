/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/TypeTraits.h"

#include <ranges>

namespace awl
{
    template <class Container>
    struct inserter : std::false_type {};

    template <std::ranges::range Container>
        requires back_insertable_sequence<Container>
    struct inserter<Container> : std::true_type
    {
        static void reserve(Container& v, size_t n)
        {
            v.reserve(n);
        }

        static void insert(Container& v, typename Container::value_type&& val)
        {
            v.push_back(std::move(val));
        }
    };

    template <class Container>
        requires insertable_sequence<Container>
    struct inserter<Container> : std::true_type
    {
        static void reserve(Container& set, size_t n)
        {
            static_cast<void>(set);
            static_cast<void>(n);
        }

        static void insert(Container& set, typename Container::value_type&& val)
        {
            set.insert(std::move(val));
        }
    };

    template <class Container>
    inline constexpr bool inserter_defined = inserter<Container>::value;
}
