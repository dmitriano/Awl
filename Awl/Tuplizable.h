/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/TupleHelpers.h"

#include <concepts>
#include <type_traits>

namespace awl
{
    // There are two ways to make type T tuplizable:
    // 1. Define T::as_tuple (if T is a user's class or struct).
    // 2. Specialize tuplizable_traits.
    // see RwHelpersTest.cpp for an example.

    template <class T, typename = void>
    struct tuplizable_traits
    {};

    template <class T> requires (!std::same_as<T, std::remove_cvref_t<T>>)
    struct tuplizable_traits<T> : tuplizable_traits<std::remove_cvref_t<T>>
    {};

    template <class T>
    constexpr auto object_as_tuple(T & val) -> decltype(tuplizable_traits<std::remove_cvref_t<T>>::tie(val))
    {
        return tuplizable_traits<std::remove_cvref_t<T>>::tie(val);
    }

    template <class T>
    constexpr auto object_as_const_tuple(const T & val) -> decltype(tuplizable_traits<std::remove_cvref_t<T>>::tie(val))
    {
        return tuplizable_traits<std::remove_cvref_t<T>>::tie(val);
    }

    template <class T>
    constexpr auto object_as_tuple(const T & val) -> decltype(object_as_const_tuple(val))
    {
        return object_as_const_tuple(val);
    }

    namespace detail
    {
        template <class T>
        concept member_tuplizable = requires(T & t, const T & ct)
        {
            t.as_tuple();
            ct.as_const_tuple();
        };
    }

    template <class T> requires std::same_as<T, std::remove_cvref_t<T>> && detail::member_tuplizable<T>
    struct tuplizable_traits<T>
    {
        static constexpr auto tie(const T & val)
        {
            return val.as_const_tuple();
        }

        static constexpr auto tie(T & val)
        {
            return val.as_tuple();
        }

        using ConstTie = decltype(tie(std::declval<const T&>()));
        using Tie = decltype(tie(std::declval<T&>()));
    };

    template <class T>
    concept tuplizable = requires(T & t, const T & ct)
    {
        typename tuplizable_traits<std::remove_cvref_t<T>>::ConstTie;
        typename tuplizable_traits<std::remove_cvref_t<T>>::Tie;
        tuplizable_traits<std::remove_cvref_t<T>>::tie(t);
        tuplizable_traits<std::remove_cvref_t<T>>::tie(ct);
    };

    template <class T>
    constexpr bool objects_equal(const T & left, const T & right)
    {
        return object_as_tuple(left) == object_as_tuple(right);
    }

    template <class T>
    constexpr bool objects_less(const T & left, const T & right)
    {
        return object_as_tuple(left) < object_as_tuple(right);
    }

    template <class T>
    constexpr bool objects_greater(const T & left, const T & right)
    {
        return object_as_tuple(left) > object_as_tuple(right);
    }

    template <class T>
    constexpr auto objects_diff(const T& left, const T& right)
    {
        return tuple_diff(object_as_tuple(left), object_as_tuple(right));
    }
}

//Used inside of a class definition for defining as_tuple() member functions.
//Theoretically all the as_tuple() overloads can be constexpr, but GCC 4.9 does not compile it.
#define AWL_TUPLIZABLE(...) \
    constexpr auto as_const_tuple() const \
    { \
        return std::tie(__VA_ARGS__); \
    } \
    constexpr auto as_tuple() const \
    { \
        return as_const_tuple(); \
    } \
    constexpr auto as_tuple() \
    { \
        return std::tie(__VA_ARGS__); \
    }

//Used outside of a class definition for specializing tuplizable_traits.
//It will become easy to do with __VA_OPT__ in C++20 but I don't see how to do this currently.
//It is not clear how to prepend VA_ARGS with 'val.' in std::tie() call.
/*
#define AWL_SERIALIZABLE_CLASS(ClassName, ...) \
    namespace awl \
    { \
        template <> \
        struct tuplizable_traits<ClassName> \
        { \
            static auto tie(const ClassName & val) \
            { \
                return std::tie(__VA_ARGS__); \
            } \
            static auto tie(ClassName & val) \
            { \
                return std::tie(__VA_ARGS__); \
            } \
            using ConstTie = decltype(tie(std::declval<const ClassName&>())); \
            using Tie = decltype(tie(std::declval<ClassName&>())); \
        }; \
    }
*/

#define AWL_BINARY_OPERATOR(ClassName, OP) \
    inline bool operator OP (const ClassName & left, const ClassName & right) \
    { \
        return awl::object_as_tuple(left) OP awl::object_as_tuple(right); \
    }

#define AWL_MEMBERWISE_EQUATABLE(ClassName) \
    AWL_BINARY_OPERATOR(ClassName, ==) \
    AWL_BINARY_OPERATOR(ClassName, !=)

#define AWL_MEMBERWISE_COMPARABLE(ClassName) \
    AWL_BINARY_OPERATOR(ClassName, <) \
    AWL_BINARY_OPERATOR(ClassName, >) \
    AWL_BINARY_OPERATOR(ClassName, <=) \
    AWL_BINARY_OPERATOR(ClassName, >=)

#define AWL_MEMBERWISE_EQUATABLE_AND_COMPARABLE(ClassName) \
    AWL_MEMBERWISE_EQUATABLE(ClassName) \
    AWL_MEMBERWISE_COMPARABLE(ClassName)
