/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <tuple>
#include <variant>
#include <type_traits>

namespace awl::mp
{
    template <class V, class T, std::size_t... index>
    constexpr bool variant_contains_impl(std::index_sequence<index...>)
    {
        return (std::is_same_v<std::variant_alternative_t<index, V>, T> || ...);
    }

    template <class V, class T>
    constexpr inline bool variant_contains = variant_contains_impl<V, T>(std::make_index_sequence<std::variant_size_v<V>>());

    static_assert(variant_contains<std::variant<int, bool, double>, int>);
    static_assert(!variant_contains<std::variant<int, bool, double>, float>);

    //Splitting std::variant

    namespace impl
    {
        template <class V>
        struct convert_empty_variant
        {
            using type = V;
        };

        template <>
        struct convert_empty_variant<std::variant<>>
        {
            using type = std::variant<std::monostate>;
        };

        template <class V>
        using convert_empty_variant_t = typename convert_empty_variant<V>::type;

        template <class V1, class V2, template <class> class Predicate, class V>
        struct split_variant;

        template <class V1, class V2, template <class> class Predicate>
        struct split_variant<V1, V2, Predicate, std::variant<>>
        {
            using matching = convert_empty_variant_t<V1>;
            using non_matching = convert_empty_variant_t<V2>;
        };

        template <class... V1s, class... V2s, template <class> class Predicate, class Head, class... Tail>
        struct split_variant<std::variant<V1s...>, std::variant<V2s...>, Predicate, std::variant<Head, Tail...>> : std::conditional_t<
            Predicate<Head>::value,
            split_variant<std::variant<V1s..., Head>, std::variant<V2s...>, Predicate, std::variant<Tail...>>,
            split_variant<std::variant<V1s...>, std::variant<V2s..., Head>, Predicate, std::variant<Tail...>>
        >
        {
        };
    }

    template <class V, template <class> class Predicate>
    using split_variant = impl::split_variant<std::variant<>, std::variant<>, Predicate, V>;

    template <class V, template <class> class Predicate>
    using filter_variant = typename split_variant<V, Predicate>::matching;

    namespace tests
    {
        using V = std::variant<bool, char, float, int, double>;
        using V1 = std::variant<bool, char, int>;
        using V2 = std::variant<float, double>;

        using result = split_variant<V, std::is_integral>;

        static_assert(std::is_same_v<typename result::matching, V1>);
        static_assert(std::is_same_v<typename result::non_matching, V2>);
    }

    //Conversion std::tuple to std::variant

    namespace impl
    {
        template <class V, class T>
        struct tuple_to_variant_convertor;

        template <class V>
        struct tuple_to_variant_convertor<V, std::tuple<>>
        {
            using result = V;
        };

        template <class... Vs, class Head, class... Tail>
        struct tuple_to_variant_convertor<std::variant<Vs...>, std::tuple<Head, Tail...>> : std::conditional_t<
            !variant_contains<std::variant<Vs...>, Head>,
            tuple_to_variant_convertor<std::variant<Vs..., Head>, std::tuple<Tail...>>,
            tuple_to_variant_convertor<std::variant<Vs...>, std::tuple<Tail...>>
        >
        {
        };
    }

    template <class T>
    using tuple_to_variant = typename impl::tuple_to_variant_convertor<std::variant<>, T>::result;

    static_assert(std::is_same_v<std::variant<int, bool, double>, tuple_to_variant<std::tuple<int, bool, double, int, bool>>>);
}
