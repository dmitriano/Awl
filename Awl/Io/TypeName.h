/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/FixedString.h"
#include "Awl/TupleHelpers.h"
#include "Awl/TypeTraits.h"
#include "Awl/Reflection.h"
#include "Awl/Decimal.h"
#include "Awl/VectorSet.h"
#include "Awl/Ring.h"

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
#include <memory>

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
                return fixed_string("0");
            }
            else
            {
                constexpr size_t N = GetNumberLength<val>();
                fixed_string<char, N> buf;

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
        static_assert(fixed_string{ "5" }.size() == 1);
        static_assert(FormatNumber<5>() == fixed_string{ "5" });
        static_assert(FormatNumber<0>() == fixed_string{ "0" });
        static_assert(FormatNumber<35>() == fixed_string{ "35" });

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
            auto suffix = helpers::GetArithmeticSize<T>() + fixed_string{ "_t" };

            if constexpr (std::is_integral_v<T>)
            {
                //Both signed and unsigned are 'int'.
                return fixed_string{ "int" } +suffix;
            }
            else
            {
                if constexpr (std::is_floating_point_v<T>)
                {
                    return fixed_string{ "float" } +suffix;
                }
                else
                {
                    static_assert(dependent_false_v<T>, "Unknown arithmetic type");
                }
            }
        }
    };

    static_assert(make_type_name<int32_t>() == fixed_string{ "int32_t" });
    static_assert(make_type_name<uint16_t>() == fixed_string{ "int16_t" });
    static_assert(make_type_name<float>() == fixed_string{ "float32_t" });

    template <class T>
    struct type_descriptor<T, std::enable_if_t<
        is_specialization_v<T, std::basic_string> ||
        is_specialization_v<T, std::vector> ||
        is_specialization_v<T, std::deque> ||
        is_specialization_v<T, std::list> ||
        is_specialization_v<T, std::set> ||
        is_specialization_v<T, std::unordered_set> ||
        is_specialization_v<T, awl::vector_set> ||
        is_specialization_v<T, awl::ring>>>
    {
        static constexpr auto name()
        {
            return fixed_string("sequence<") + make_type_name<typename T::value_type>() + fixed_string(">");
        }
    };

    static_assert(make_type_name<std::string>() == fixed_string{ "sequence<int8_t>" });
    //wstring is int32_t in GCC and uint16_t in MSVC
    //static_assert(make_type_name<std::wstring>() == fixed_string{ "sequence<int16_t>" });
    static_assert(make_type_name<std::vector<int32_t>>() == fixed_string{ "sequence<int32_t>" });
    static_assert(make_type_name<std::vector<std::list<uint64_t>>>() == fixed_string{ "sequence<sequence<int64_t>>" });

    template <class T>
    struct type_descriptor<T, std::enable_if_t<
        is_specialization_v<T, std::map> ||
        is_specialization_v<T, std::unordered_map>>>
    {
        static constexpr auto name()
        {
            return fixed_string("map<") + make_type_name<typename T::key_type>() + fixed_string(", ") + make_type_name<typename T::mapped_type>() + fixed_string(">");
        }
    };

    static_assert(make_type_name<std::map<int32_t, int64_t>>() == fixed_string{ "map<int32_t, int64_t>" });

    template <class Clock, class Duration>
    struct type_descriptor<std::chrono::time_point<Clock, Duration>>
    {
        static constexpr auto name()
        {
            return make_type_name<int64_t>();
        }
    };

    static_assert(make_type_name<std::chrono::system_clock::time_point>() == fixed_string{ "int64_t" });

    template <class T>
    struct type_descriptor<std::optional<T>>
    {
        static constexpr auto name()
        {
            return fixed_string("optional<") + make_type_name<T>() + fixed_string(">");
        }
    };

    static_assert(make_type_name<std::optional<std::string>>() == fixed_string("optional<sequence<int8_t>>"));

    template<class T, std::size_t N>
    struct type_descriptor<std::array<T, N>>
    {
        static constexpr auto name()
        {
            return fixed_string("array<") + make_type_name<T>() + fixed_string(", ") + helpers::FormatNumber<N>() + fixed_string(">");
        }
    };

    static_assert(make_type_name<std::array<uint8_t, 5>>() == fixed_string{ "array<int8_t, 5>" });

    template <class T>
    struct type_descriptor<T, std::enable_if_t<is_reflectable_v<T>>>
    {
        static constexpr auto name()
        {
            return fixed_string("struct");
        }
    };

    template <class... Ts>
    struct type_descriptor<std::variant<Ts...>>
    {
        static constexpr auto name()
        {
            return fixed_string("variant<") + ((make_type_name<Ts>() + fixed_string(", ")) + ...) + fixed_string(">");
        }
    };

    static_assert(make_type_name<std::variant<int32_t, int64_t>>() == fixed_string{ "variant<int32_t, int64_t, >" });

    template <typename UInt, uint8_t exp_len, template <typename, uint8_t> class DataTemplate>
    struct type_descriptor<decimal<UInt, exp_len, DataTemplate>>
    {
        static constexpr auto name()
        {
            if constexpr (std::is_same_v<UInt, uint64_t> && exp_len == 4)
            {
                //Compatibility with the previous version.
                return fixed_string("decimal");
            }
            else
            {
                return fixed_string("decimal<") + make_type_name<UInt>() + fixed_string(", ") + helpers::FormatNumber<exp_len>() + fixed_string(">");
            }
        }
    };

    static_assert(make_type_name<decimal<uint64_t, 5>>() == fixed_string{ "decimal<int64_t, 5>" });

    // Pointers can be implemented in the same was as std::optional.

    namespace helpers
    {
        template <class T>
        constexpr auto make_pointer_type_name()
        {
            return fixed_string("pointer<") + make_type_name<T>() + fixed_string(">");
        }
    }

    template <class T>
    struct type_descriptor<std::shared_ptr<T>>
    {
        static constexpr auto name()
        {
            return helpers::make_pointer_type_name<T>();
        }
    };

    template <class T>
    struct type_descriptor<std::unique_ptr<T>>
    {
        static constexpr auto name()
        {
            return helpers::make_pointer_type_name<T>();
        }
    };

    template <class T>
    struct type_descriptor<T *>
    {
        static constexpr auto name()
        {
            return helpers::make_pointer_type_name<T>();
        }
    };

    template <class T> requires std::is_enum_v<T>
    struct type_descriptor<T>
    {
        static constexpr auto name()
        {
            return make_type_name<std::underlying_type_t<T>>();
        }
    };
}
