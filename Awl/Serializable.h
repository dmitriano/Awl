#pragma once

#include "Awl/TupleHelpers.h"
#include "Awl/Lang.h"

#include <type_traits>

namespace awl
{
    template <class T>
    inline AWL_CONSTEXPR auto object_as_tuple(T & val)
    {
        return val.as_tuple();
    }

    template <class T>
    inline AWL_CONSTEXPR auto object_as_tuple(const T & val)
    {
        return val.as_tuple();
    }

    template <class T>
    inline AWL_CONSTEXPR bool objects_equal(const T & left, const T & right)
    {
        return object_as_tuple(left) == object_as_tuple(right);
    }

    template <class T>
    inline AWL_CONSTEXPR bool objects_less(const T & left, const T & right)
    {
        return object_as_tuple(left) < object_as_tuple(right);
    }

    template <class T>
    inline AWL_CONSTEXPR bool objects_greater(const T & left, const T & right)
    {
        return object_as_tuple(left) > object_as_tuple(right);
    }
}

//Used inside of a class definition for defining as_tuple() member functions.
//Theoretically all the as_tuple() overloads can be constexpr, but GCC 4.9 does not compile it.
#define AWL_SERIALIZABLE(...) \
    AWL_CONSTEXPR auto as_const_tuple() const \
    { \
        return std::tie(__VA_ARGS__); \
    } \
    AWL_CONSTEXPR auto as_tuple() const \
    { \
        return as_const_tuple(); \
    } \
    AWL_CONSTEXPR auto as_tuple() \
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
        inline auto object_as_tuple(const ClassName & val) \
        { \
            return std::tie(__VA_ARGS__); \
        } \
        template <> \
        inline auto object_as_tuple(ClassName & val) \
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
