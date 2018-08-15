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

    static_assert(is_collection<std::vector<int>>::value, "std::vector<int> is not a container.");

    template <typename T, typename S>
    struct is_string
    {
        static const bool value = false;
    };

    template <class T, class Traits, class Alloc>
    struct is_string<T, std::basic_string<T, Traits, Alloc>>
    {
        static const bool value = true;
    };

    static_assert(is_string<char, std::string>::value, "is_string<char, std::string>::value is false.");
    static_assert(!is_string<char, std::wstring>::value, "is_string<char, std::wstring>::value is true.");
    static_assert(!is_string<wchar_t, std::string>::value, "is_string<wchar_t, std::string>::value is true.");
    static_assert(is_string<wchar_t, std::wstring>::value, "is_string<wchar_t, std::wstring>::value is false.");
};