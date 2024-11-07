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
#include <atomic>
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

    template <class T>
    struct type_descriptor;
    
    template <class T>
    constexpr auto make_type_name()
    {
        return type_descriptor<T>::name();
    }
    
    template <class T> requires std::is_arithmetic_v<T>
    struct type_descriptor<T>
    {
        using inner_tuple = std::tuple<>;

        static constexpr auto name()
        {
            auto suffix = helpers::GetArithmeticSize<T>() + fixed_string{ "_t" };

            if constexpr (std::is_integral_v<T>)
            {
                //Both signed and unsigned are 'int'.
                return fixed_string{ "int" } + suffix;
            }
            else
            {
                if constexpr (std::is_floating_point_v<T>)
                {
                    return fixed_string{ "float" } + suffix;
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

    template <std::ranges::range Coll>
    struct type_descriptor<Coll>
    {
        using inner_tuple = std::tuple<std::ranges::range_value_t<Coll>>;

        static constexpr auto name()
        {
            return fixed_string("sequence<") + make_type_name<std::ranges::range_value_t<Coll>>() + fixed_string(">");
        }
    };

    static_assert(make_type_name<std::string>() == fixed_string{ "sequence<int8_t>" });
    //wstring is int32_t in GCC and uint16_t in MSVC
    //static_assert(make_type_name<std::wstring>() == fixed_string{ "sequence<int16_t>" });
    static_assert(make_type_name<std::vector<int32_t>>() == fixed_string{ "sequence<int32_t>" });
    static_assert(make_type_name<std::vector<std::list<uint64_t>>>() == fixed_string{ "sequence<sequence<int64_t>>" });

    template <class First, class Second>
    struct type_descriptor<std::pair<First, Second>>
    {
        using inner_tuple = std::tuple<std::decay_t<First>, std::decay_t<Second>>;

        static constexpr auto name()
        {
            return fixed_string("pair<") + make_type_name<std::decay_t<First>>() + fixed_string(", ") + make_type_name<std::decay_t<Second>>() + fixed_string(">");
        }
    };
        
    static_assert(make_type_name<std::map<int32_t, int64_t>>() == fixed_string{ "sequence<pair<int32_t, int64_t>>" });

    template <class Clock, class Duration>
    struct type_descriptor<std::chrono::time_point<Clock, Duration>>
    {
        using inner_tuple = std::tuple<int64_t>;

        static constexpr auto name()
        {
            return make_type_name<int64_t>();
        }
    };

    static_assert(make_type_name<std::chrono::system_clock::time_point>() == fixed_string{ "int64_t" });

    template <class T>
    struct type_descriptor<std::optional<T>>
    {
        using inner_tuple = std::tuple<T>;

        static constexpr auto name()
        {
            return fixed_string("optional<") + make_type_name<T>() + fixed_string(">");
        }
    };

    static_assert(make_type_name<std::optional<std::string>>() == fixed_string("optional<sequence<int8_t>>"));

    template <class T>
    struct type_descriptor<std::atomic<T>>
    {
        using inner_tuple = std::tuple<T>;

        static constexpr auto name()
        {
            return make_type_name<T>();
        }
    };

    static_assert(make_type_name<std::atomic<int8_t>>() == fixed_string("int8_t"));

    template<class T, std::size_t N>
    struct type_descriptor<std::array<T, N>>
    {
        using inner_tuple = std::tuple<T>;

        static constexpr auto name()
        {
            return fixed_string("array<") + make_type_name<T>() + fixed_string(", ") + helpers::FormatNumber<N>() + fixed_string(">");
        }
    };

    static_assert(make_type_name<std::array<uint8_t, 5>>() == fixed_string{ "array<int8_t, 5>" });

    template <class T> requires is_reflectable_v<T>
    struct type_descriptor<T>
    {
        using inner_tuple = typename tuplizable_traits<T>::Tie;

        static constexpr auto name()
        {
            return fixed_string("struct");
        }
    };

    template <class... Ts>
    struct type_descriptor<std::variant<Ts...>>
    {
        using inner_tuple = std::tuple<Ts...>;

        static constexpr auto name()
        {
            return fixed_string("variant<") + ((make_type_name<Ts>() + fixed_string(", ")) + ...) + fixed_string(">");
        }
    };

    static_assert(make_type_name<std::variant<int32_t, int64_t>>() == fixed_string{ "variant<int32_t, int64_t, >" });

    template <typename UInt, uint8_t exp_len, template <typename, uint8_t> class DataTemplate>
    struct type_descriptor<decimal<UInt, exp_len, DataTemplate>>
    {
        using inner_tuple = std::tuple<>;

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
        using inner_tuple = std::tuple<T>;

        static constexpr auto name()
        {
            return helpers::make_pointer_type_name<T>();
        }
    };

    template <class T>
    struct type_descriptor<std::unique_ptr<T>>
    {
        using inner_tuple = std::tuple<T>;

        static constexpr auto name()
        {
            return helpers::make_pointer_type_name<T>();
        }
    };

    template <class T>
    struct type_descriptor<T *>
    {
        using inner_tuple = std::tuple<T>;

        static constexpr auto name()
        {
            return helpers::make_pointer_type_name<T>();
        }
    };

    template <class T> requires std::is_enum_v<T>
    struct type_descriptor<T>
    {
        using inner_tuple = std::tuple<>;

        static constexpr auto name()
        {
            return make_type_name<std::underlying_type_t<T>>();
        }
    };
}
