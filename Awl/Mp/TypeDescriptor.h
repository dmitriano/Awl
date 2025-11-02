/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/TupleHelpers.h"
#include "Awl/TypeTraits.h"
#include "Awl/Reflection.h"
#include "Awl/Decimal.h"

#include <type_traits>
#include <tuple>
#include <utility> 
#include <array>
#include <chrono>
#include <optional>
#include <atomic>
#include <variant>
#include <memory>

namespace awl::mp
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
        constexpr std::string FormatNumber()
        {
            if constexpr (val == 0)
            {
                return std::string("0");
            }
            else
            {
                constexpr size_t N = GetNumberLength<val>();
                std::string buf;
                buf.resize(N);

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
        static_assert(std::string("5").size() == 1);
        static_assert(FormatNumber<5>() == std::string("5"));
        static_assert(FormatNumber<0>() == std::string("0"));
        static_assert(FormatNumber<35>() == std::string("35"));

        template <class T, std::enable_if_t<std::is_arithmetic<T>{}, bool > = true >
        constexpr auto GetArithmeticSize()
        {
            return FormatNumber<sizeof(T) * 8>();
        }
    }

    template <class T>
    struct type_descriptor;
    
    template <class T>
    constexpr std::string make_type_name()
    {
        return type_descriptor<T>::name();
    }
    
    template <class T> requires std::is_arithmetic_v<T>
    struct type_descriptor<T>
    {
        using inner_tuple = std::tuple<>;

        static constexpr std::string name()
        {
            auto suffix = helpers::GetArithmeticSize<T>() + std::string("_t");

            if constexpr (std::is_integral_v<T>)
            {
                //Both signed and unsigned are 'int'.
                return std::string("int") + suffix;
            }
            else
            {
                if constexpr (std::is_floating_point_v<T>)
                {
                    return std::string("float") + suffix;
                }
                else
                {
                    static_assert(dependent_false_v<T>, "Unknown arithmetic type");
                }
            }
        }
    };

    static_assert(make_type_name<int32_t>() == std::string("int32_t"));
    static_assert(make_type_name<uint16_t>() == std::string("int16_t"));
    static_assert(make_type_name<float>() == std::string("float32_t"));

    template <std::ranges::range Coll>
    struct type_descriptor<Coll>
    {
        using inner_tuple = std::tuple<std::ranges::range_value_t<Coll>>;

        static constexpr std::string name()
        {
            return std::string("sequence<") + make_type_name<std::ranges::range_value_t<Coll>>() + std::string(">");
        }
    };

    template <class First, class Second>
    struct type_descriptor<std::pair<First, Second>>
    {
        using inner_tuple = std::tuple<std::decay_t<First>, std::decay_t<Second>>;

        static constexpr std::string name()
        {
            return std::string("pair<") + make_type_name<std::decay_t<First>>() + std::string(", ") + make_type_name<std::decay_t<Second>>() + std::string(">");
        }
    };
        
    template <class Clock, class Duration>
    struct type_descriptor<std::chrono::time_point<Clock, Duration>>
    {
        using inner_tuple = std::tuple<int64_t>;

        static constexpr std::string name()
        {
            return make_type_name<int64_t>();
        }
    };

    static_assert(make_type_name<std::chrono::system_clock::time_point>() == std::string("int64_t"));

    template <class T>
    struct type_descriptor<std::optional<T>>
    {
        using inner_tuple = std::tuple<T>;

        static constexpr std::string name()
        {
            return std::string("optional<") + make_type_name<T>() + std::string(">");
        }
    };

    template <class T>
    struct type_descriptor<std::atomic<T>>
    {
        using inner_tuple = std::tuple<T>;

        static constexpr std::string name()
        {
            return make_type_name<T>();
        }
    };

    static_assert(make_type_name<std::atomic<int8_t>>() == std::string("int8_t"));

    template<class T, std::size_t N>
    struct type_descriptor<std::array<T, N>>
    {
        using inner_tuple = std::tuple<T>;

        static constexpr std::string name()
        {
            return std::string("array<") + make_type_name<T>() + std::string(", ") + helpers::FormatNumber<N>() + std::string(">");
        }
    };

    static_assert(make_type_name<std::array<uint8_t, 5>>() == std::string("array<int8_t, 5>"));

    template <class T> requires is_reflectable_v<T>
    struct type_descriptor<T>
    {
        using inner_tuple = typename tuplizable_traits<T>::Tie;

        static constexpr std::string name()
        {
            return std::string("struct");
        }
    };

    template <class... Ts>
    struct type_descriptor<std::variant<Ts...>>
    {
        using inner_tuple = std::tuple<Ts...>;

        static constexpr std::string name()
        {
            return std::string("variant<") + ((make_type_name<Ts>() + std::string(", ")) + ...) + std::string(">");
        }
    };

    static_assert(make_type_name<std::variant<int32_t, int64_t>>() == std::string("variant<int32_t, int64_t, >"));

    template <class... Ts>
    struct type_descriptor<std::tuple<Ts...>>
    {
        using inner_tuple = std::tuple<Ts...>;

        static constexpr std::string name()
        {
            return std::string("tuple<") + ((make_type_name<Ts>() + std::string(", ")) + ...) + std::string(">");
        }
    };

    static_assert(make_type_name<std::tuple<int32_t, int64_t>>() == std::string("tuple<int32_t, int64_t, >"));

    template <typename UInt, uint8_t exp_len, template <typename, uint8_t> class DataTemplate>
    struct type_descriptor<decimal<UInt, exp_len, DataTemplate>>
    {
        using inner_tuple = std::tuple<>;

        static constexpr std::string name()
        {
            if constexpr (std::is_same_v<UInt, uint64_t> && exp_len == 4)
            {
                //Compatibility with the previous version.
                return std::string("decimal");
            }
            else
            {
                return std::string("decimal<") + make_type_name<UInt>() + std::string(", ") + helpers::FormatNumber<exp_len>() + std::string(">");
            }
        }
    };

    static_assert(make_type_name<decimal<uint64_t, 5>>() == std::string("decimal<int64_t, 5>"));

    // Pointers can be implemented in the same was as std::optional.

    namespace helpers
    {
        template <class T>
        constexpr auto make_pointer_type_name()
        {
            return std::string("pointer<") + make_type_name<T>() + std::string(">");
        }
    }

    template <class T>
    struct type_descriptor<std::shared_ptr<T>>
    {
        using inner_tuple = std::tuple<T>;

        static constexpr std::string name()
        {
            return helpers::make_pointer_type_name<T>();
        }
    };

    template <class T>
    struct type_descriptor<std::unique_ptr<T>>
    {
        using inner_tuple = std::tuple<T>;

        static constexpr std::string name()
        {
            return helpers::make_pointer_type_name<T>();
        }
    };

    template <class T>
    struct type_descriptor<T *>
    {
        using inner_tuple = std::tuple<T>;

        static constexpr std::string name()
        {
            return helpers::make_pointer_type_name<T>();
        }
    };

    template <class T> requires std::is_enum_v<T>
    struct type_descriptor<T>
    {
        using inner_tuple = std::tuple<>;

        static constexpr std::string name()
        {
            return make_type_name<std::underlying_type_t<T>>();
        }
    };
}

#define DEFINE_TRIVIAL_TYPE_DESRIPTOR(type_name) \
namespace awl::mp \
{\
    template <> \
    struct type_descriptor<type_name> \
    { \
        static constexpr std::string name() \
        { \
            return #type_name; \
        } \
    }; \
}
