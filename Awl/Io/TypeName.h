#pragma once

#include "Awl/FixedString.h"
#include "Awl/TupleHelpers.h"
#include "Awl/Stringizable.h"

#include <string>
#include <vector>
#include <array>
#include <list>
#include <bitset>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <chrono>
#include <type_traits>
#include <tuple>
#include <utility> 
#include <optional>
#include <variant>

namespace awl::io
{
    namespace helpers
    {
        template <size_t val>
        constexpr size_t GetNumberLength()
        {
            size_t i = 0;
            size_t n = val;

            while (n != 0)
            {
                n /= 10;
                ++i;
            }

            return i;
        }

        static_assert(GetNumberLength<0>() == 0);
        static_assert(GetNumberLength<5>() == 1);
        static_assert(GetNumberLength<35>() == 2);

        template <size_t val>
        constexpr auto FormatNumber()
        {
            if constexpr (val == 0)
            {
                return FixedString("0");
            }
            else
            {
                constexpr size_t N = GetNumberLength<val>();
                FixedString<N> buf;

                size_t n = val;
                for (int i = N - 1; i >= 0; --i)
                {
                    buf[i] = static_cast<char>(n % 10 + 48);
                    n /= 10;
                }

                return buf;
            }
        }

        static_assert(FormatNumber<5>().size() == 1);
        static_assert(FixedString{ "5" }.size() == 1);
        static_assert(FormatNumber<5>() == FixedString{ "5" });
        static_assert(FormatNumber<0>() == FixedString{ "0" });
        static_assert(FormatNumber<35>() == FixedString{ "35" });

        template <class T, std::enable_if_t<std::is_arithmetic<T>{}, bool > = true >
        constexpr auto GetArithmeticSize()
        {
            return FormatNumber<sizeof(T) * 8>();
        }
    }

    template <class T, class Enable = void>
    struct type_descriptor {};
    
    template <class T>
    constexpr auto make_type_name()
    {
        return type_descriptor<T>::name();
    }
    
    template <class T>
    struct type_descriptor<T, std::enable_if_t<std::is_arithmetic_v<T>>>
    {
        static constexpr auto name()
        {
            auto suffix = helpers::GetArithmeticSize<T>() + FixedString{ "_t" };

            if constexpr (std::is_integral_v<T>)
            {
                //Both signed and unsigned are 'int'.
                return FixedString{ "int" } +suffix;
            }
            else
            {
                if constexpr (std::is_floating_point_v<T>)
                {
                    return FixedString{ "float" } +suffix;
                }
                else
                {
                    static_assert(dependent_false_v<T>, "Unknown arithmetic type");
                }
            }
        }
    };


    static_assert(make_type_name<int32_t>() == FixedString{ "int32_t" });
    static_assert(make_type_name<uint16_t>() == FixedString{ "int16_t" });
    static_assert(make_type_name<float>() == FixedString{ "float32_t" });

    template<typename Test, template<typename...> class Ref>
    struct is_specialization : std::false_type {};

    template<template<typename...> class Ref, typename... Args>
    struct is_specialization<Ref<Args...>, Ref> : std::true_type {};

    template<typename Test, template<typename...> class Ref>
    constexpr bool is_specialization_v = is_specialization<Test, Ref>::value;

    template <class T>
    struct type_descriptor<T, std::enable_if_t<
        is_specialization_v<T, std::basic_string> ||
        is_specialization_v<T, std::vector> ||
        is_specialization_v<T, std::list> ||
        is_specialization_v<T, std::set> ||
        is_specialization_v<T, std::unordered_set>>>
    {
        static constexpr auto name()
        {
            return FixedString("sequence<") + make_type_name<typename T::value_type>() + FixedString(">");
        }
    };

    static_assert(make_type_name<std::string>() == FixedString{ "sequence<int8_t>" });
    //wstring is int32_t in GCC and uint16_t in MSVC
    //static_assert(make_type_name<std::wstring>() == FixedString{ "sequence<int16_t>" });
    static_assert(make_type_name<std::vector<int32_t>>() == FixedString{ "sequence<int32_t>" });
    static_assert(make_type_name<std::vector<std::list<uint64_t>>>() == FixedString{ "sequence<sequence<int64_t>>" });

    template <class T>
    struct type_descriptor<T, std::enable_if_t<
        is_specialization_v<T, std::map> ||
        is_specialization_v<T, std::unordered_map>>>
    {
        static constexpr auto name()
        {
            return FixedString("map<") + make_type_name<typename T::key_type>() + FixedString(", ") + make_type_name<typename T::mapped_type>() + FixedString(">");
        }
    };

    static_assert(make_type_name<std::map<int32_t, int64_t>>() == FixedString{ "map<int32_t, int64_t>" });

    template <class T>
    struct type_descriptor<T, std::enable_if_t<is_specialization_v<T, std::chrono::time_point>>>
    {
        static constexpr auto name()
        {
            return make_type_name<int64_t>();
        }
    };

    static_assert(make_type_name<std::chrono::system_clock::time_point>() == FixedString{ "int64_t" });

    template <class T>
    struct type_descriptor<T, std::enable_if_t<is_specialization_v<T, std::optional>>>
    {
        static constexpr auto name()
        {
            return FixedString("optional<") + make_type_name<typename T::value_type>() + FixedString(">");
        }
    };

    static_assert(make_type_name<std::optional<std::string>>() == FixedString("optional<sequence<int8_t>>"));

    template<class T>
    struct is_array : std::is_array<T> {};
    template<class T, std::size_t N>
    struct is_array<std::array<T, N>> : std::true_type {};
    // optional:
    template<class T>
    struct is_array<T const> : is_array<T> {};
    template<class T>
    struct is_array<T volatile> : is_array<T> {};
    template<class T>
    struct is_array<T volatile const> : is_array<T> {};

    template<class T>
    constexpr bool is_array_v = is_array<T>::value;

    template <class T>
    struct type_descriptor<T, std::enable_if_t<is_array_v<T>>>
    {
        static constexpr auto name()
        {
            return FixedString("array<") + make_type_name<typename T::value_type>() + FixedString(", ") + helpers::FormatNumber < T{}.size() > () + FixedString(">");
        }
    };

    static_assert(make_type_name<std::array<uint8_t, 5>>() == FixedString{ "array<int8_t, 5>" });

    template <class T>
    struct type_descriptor<T, std::enable_if_t<is_stringizable_v<T>>>
    {
        static constexpr auto name()
        {
            return FixedString("struct");
        }
    };

    template <class... Ts>
    struct type_descriptor<std::variant<Ts...>>
    {
        static constexpr auto name()
        {
            return FixedString("variant<") + ((type_descriptor<Ts>::name() + FixedString(", ")) + ...) + FixedString(">");
        }
    };

    static_assert(make_type_name<std::variant<int32_t, int64_t>>() == FixedString{ "variant<int32_t, int64_t, >" });
}
