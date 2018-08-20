#pragma once

#include <string>
#include <vector>
#include <set>

namespace awl
{
    namespace testing
    {
        template <typename T>
        struct is_collection : std::false_type {};

        template <class T, class Alloc>
        struct is_collection<std::vector<T, Alloc>> : std::true_type {};

        template <class T, class Compare, class Alloc>
        struct is_collection<std::set<T, Compare, Alloc>> : std::true_type {};

        static_assert(is_collection<std::vector<int>>::value, "std::vector<int> is not a collection.");

        template <typename T, typename S>
        struct is_string : std::false_type {};

        template <class T, class Traits, class Alloc>
        struct is_string<T, std::basic_string<T, Traits, Alloc>> : std::true_type {};

        static_assert(is_string<char, std::string>::value, "is_string<char, std::string>::value is false.");
        static_assert(!is_string<char, std::wstring>::value, "is_string<char, std::wstring>::value is true.");
        static_assert(!is_string<wchar_t, std::string>::value, "is_string<wchar_t, std::string>::value is true.");
        static_assert(is_string<wchar_t, std::wstring>::value, "is_string<wchar_t, std::wstring>::value is false.");
    }
};