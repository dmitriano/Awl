/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Text.h"
#include "Awl/TupleHelpers.h"
#include "Awl/TypeTraits.h"
#include "Awl/Stringizable.h"
#include "Awl/Decimal.h"

#include <string>
#include <vector>
#include <deque>
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
                return text("0");
            }
            else
            {
                constexpr size_t N = GetNumberLength<val>();
                text<char, N> buf;

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
        static_assert(text{ "5" }.size() == 1);
        static_assert(FormatNumber<5>() == text{ "5" });
        static_assert(FormatNumber<0>() == text{ "0" });
        static_assert(FormatNumber<35>() == text{ "35" });

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
            auto suffix = helpers::GetArithmeticSize<T>() + text{ "_t" };

            if constexpr (std::is_integral_v<T>)
            {
                //Both signed and unsigned are 'int'.
                return text{ "int" } +suffix;
            }
            else
            {
                if constexpr (std::is_floating_point_v<T>)
                {
                    return text{ "float" } +suffix;
                }
                else
                {
                    static_assert(dependent_false_v<T>, "Unknown arithmetic type");
                }
            }
        }
    };

    static_assert(make_type_name<int32_t>() == text{ "int32_t" });
    static_assert(make_type_name<uint16_t>() == text{ "int16_t" });
    static_assert(make_type_name<float>() == text{ "float32_t" });

    template <class T>
    struct type_descriptor<T, std::enable_if_t<
        is_specialization_v<T, std::basic_string> ||
        is_specialization_v<T, std::vector> ||
        is_specialization_v<T, std::deque> ||
        is_specialization_v<T, std::list> ||
        is_specialization_v<T, std::set> ||
        is_specialization_v<T, std::unordered_set>>>
    {
        static constexpr auto name()
        {
            return text("sequence<") + make_type_name<typename T::value_type>() + text(">");
        }
    };

    static_assert(make_type_name<std::string>() == text{ "sequence<int8_t>" });
    //wstring is int32_t in GCC and uint16_t in MSVC
    //static_assert(make_type_name<std::wstring>() == text{ "sequence<int16_t>" });
    static_assert(make_type_name<std::vector<int32_t>>() == text{ "sequence<int32_t>" });
    static_assert(make_type_name<std::vector<std::list<uint64_t>>>() == text{ "sequence<sequence<int64_t>>" });

    template <class T>
    struct type_descriptor<T, std::enable_if_t<
        is_specialization_v<T, std::map> ||
        is_specialization_v<T, std::unordered_map>>>
    {
        static constexpr auto name()
        {
            return text("map<") + make_type_name<typename T::key_type>() + text(", ") + make_type_name<typename T::mapped_type>() + text(">");
        }
    };

    static_assert(make_type_name<std::map<int32_t, int64_t>>() == text{ "map<int32_t, int64_t>" });

    template <class T>
    struct type_descriptor<T, std::enable_if_t<is_specialization_v<T, std::chrono::time_point>>>
    {
        static constexpr auto name()
        {
            return make_type_name<int64_t>();
        }
    };

    static_assert(make_type_name<std::chrono::system_clock::time_point>() == text{ "int64_t" });

    template <class T>
    struct type_descriptor<T, std::enable_if_t<is_specialization_v<T, std::optional>>>
    {
        static constexpr auto name()
        {
            return text("optional<") + make_type_name<typename T::value_type>() + text(">");
        }
    };

    static_assert(make_type_name<std::optional<std::string>>() == text("optional<sequence<int8_t>>"));

    template<class T, std::size_t N>
    struct type_descriptor<std::array<T, N>>
    {
        static constexpr auto name()
        {
            return text("array<") + make_type_name<T>() + text(", ") + helpers::FormatNumber<N>() + text(">");
        }
    };

    static_assert(make_type_name<std::array<uint8_t, 5>>() == text{ "array<int8_t, 5>" });

    template <class T>
    struct type_descriptor<T, std::enable_if_t<is_stringizable_v<T>>>
    {
        static constexpr auto name()
        {
            return text("struct");
        }
    };

    template <class... Ts>
    struct type_descriptor<std::variant<Ts...>>
    {
        static constexpr auto name()
        {
            return text("variant<") + ((make_type_name<Ts>() + text(", ")) + ...) + text(">");
        }
    };

    static_assert(make_type_name<std::variant<int32_t, int64_t>>() == text{ "variant<int32_t, int64_t, >" });

    template<>
    struct type_descriptor<awl::decimal>
    {
        static constexpr auto name()
        {
            return text("decimal");
        }
    };

    static_assert(make_type_name<awl::decimal>() == text{ "decimal" });
}
