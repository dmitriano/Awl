/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/TupleHelpers.h"

#include <type_traits>

namespace awl
{
    // There are two ways to make type T tuplizable:
    // 1. Define T::as_tuple (if T is a user's class or struct).
    // 2. Define object_as_const_tuple and object_as_tuple.
    // see RwHelpersTest.cpp for an example.

    template <class T>
    concept self_tuplizable = requires(T& t)
    {
        std::as_const(t).as_const_tuple();
        t.as_tuple;
    };

    template <class T> requires self_tuplizable<T>
    constexpr auto object_as_tuple(T & val)
    {
        return val.as_tuple();
    }

    template <class T> requires self_tuplizable<T>
    constexpr auto object_as_const_tuple(const T & val)
    {
        return val.as_const_tuple();
    }

    template <class T> requires self_tuplizable<T>
    constexpr auto object_as_tuple(const T & val)
    {
        return object_as_const_tuple(val);
    }

    template <class T>
    concept tuplizable = requires(T& t)
    {
        object_as_const_tuple(std::as_const(t));
        object_as_tuple(t);
    };

    template <class T> requires tuplizable<T>
    constexpr bool objects_equal(const T & left, const T & right)
    {
        return object_as_tuple(left) == object_as_tuple(right);
    }

    template <class T> requires tuplizable<T>
    constexpr bool objects_less(const T & left, const T & right)
    {
        return object_as_tuple(left) < object_as_tuple(right);
    }

    template <class T> requires tuplizable<T>
    constexpr bool objects_greater(const T & left, const T & right)
    {
        return object_as_tuple(left) > object_as_tuple(right);
    }

    template <class T>
    struct tuplizable_traits
    {
        using ConstTie = decltype(object_as_const_tuple(std::declval<const T&>()));
        using Tie = decltype(object_as_tuple(std::declval<T&>()));
    };
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

//There also can be 
//  friend auto awl::object_as_tuple<ClassName>(ClassName &);
//but this also does not compile with GCC 4.9, but compiles with VS2017.

//Used outside of a class definition for defining object_as_tuple functions.
//It will become easy to do with __VA_OPT__ in C++20 but I don't see how to do this currently.
//Is not clear how to prepend VA_ARGS with 'val.' in std::tie() call.
/*
#define AWL_SERIALIZABLE_CLASS(ClassName, ...) \
    namespace awl \
    { \
        template <> \
        auto object_as_tuple(const ClassName & val) \
        { \
            return std::tie(__VA_ARGS__); \
        } \
        template <> \
        auto object_as_tuple(ClassName & val) \
        { \
            return std::tie(__VA_ARGS__); \
        } \
    }
*/

#define AWL_BINARY_OPERATOR(ClassName, OP) \
    inline bool operator OP (const ClassName & left, const ClassName & right) \
    { \
        return awl::object_as_const_tuple(left) OP awl::object_as_const_tuple(right); \
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
