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
    // 2. Define object_as_const_tuple and object_as_tuple and specialize is_tuplizable<T>.
    // see RwHelpersTest.cpp for an example.

    template <class T>
    constexpr auto object_as_tuple(T & val)
    {
        return val.as_tuple();
    }

    template <class T>
    constexpr auto object_as_const_tuple(const T & val)
    {
        return val.as_const_tuple();
    }

    template <class T>
    constexpr auto object_as_tuple(const T & val)
    {
        return object_as_const_tuple(val);
    }

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

    // It can be a concept, but we can't specialize it for custom types
    // we do with inline variables.
    /*
    template <class T>
    concept tuplizable = requires(T & t)
    {
        t.as_const_tuple();
        t.as_tuple;
    };
    */

    template <class T, typename = void>
    struct is_tuplizable : std::false_type {};

    template <class T>
    struct is_tuplizable<T, std::void_t<decltype(std::declval<T>().as_const_tuple())>> : std::true_type {};

    template <class T>
    inline constexpr bool is_tuplizable_v = is_tuplizable<T>::value;

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
