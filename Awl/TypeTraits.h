#pragma once

#include <string>
#include <vector>
#include <set>

namespace awl
{
    template <typename T>
    struct is_collection
    {
        static const bool value = false;
    };

    template <class T, class Alloc>
    struct is_collection<std::vector<T, Alloc>>
    {
        static const bool value = true;
    };

    template <class T, class Compare, class Alloc>
    struct is_collection<std::set<T, Compare, Alloc>>
    {
        static const bool value = true;
    };

    template <typename T>
    struct is_string
    {
        static const bool value = false;
    };

    template <class T, class Traits, class Alloc>
    struct is_string<std::basic_string<T, Traits, Alloc>>
    {
        static const bool value = true;
    };
}
