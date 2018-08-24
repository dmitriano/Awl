#pragma once

#include "Awl/TupleHelpers.h"

#include <type_traits>

namespace awl
{
    template <class T>
    inline auto object_as_tuple(T & val)
    {
        return val.as_tuple();
    }

    template <class T>
    inline auto object_as_tuple(const T & val)
    {
        return val.as_tuple();
    }

    template <class T>
    inline bool objects_equal(const T & left, const T & right)
    {
        return object_as_tuple(left) == object_as_tuple(right);
    }

    template <class T>
    inline bool objects_less(const T & left, const T & right)
    {
        return object_as_tuple(left) < object_as_tuple(right);
    }

    template <class T>
    inline bool objects_greater(const T & left, const T & right)
    {
        return object_as_tuple(left) > object_as_tuple(right);
    }
}

//Used inside of a class definition for defining as_tuple() member functions.
#define AWL_SERIALIZABLE(...) \
    auto as_const_tuple() const \
    { \
        return std::tie(__VA_ARGS__); \
    } \
    auto as_tuple() const \
    { \
        return as_const_tuple(); \
    } \
    auto as_tuple() \
    { \
        return std::tie(__VA_ARGS__); \
    }

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

#define AWL_SERIALIZABLE_CLASS_1(ClassName, m1) \
    namespace awl \
    { \
        template <> \
        inline auto object_as_tuple(const ClassName & val) \
        { \
            return std::tie(val.m1); \
        } \
        template <> \
        inline auto object_as_tuple(ClassName & val) \
        { \
            return std::tie(val.m1); \
        } \
    }

#define AWL_SERIALIZABLE_CLASS_2(ClassName, m1, m2) \
    namespace awl \
    { \
        template <> \
        inline auto object_as_tuple(const ClassName & val) \
        { \
            return std::tie(val.m1, val.m2); \
        } \
        template <> \
        inline auto object_as_tuple(ClassName & val) \
        { \
            return std::tie(val.m1, val.m2); \
        } \
    }

#define AWL_SERIALIZABLE_CLASS_3(ClassName, m1, m2, m3) \
    namespace awl \
    { \
        template <> \
        inline auto object_as_tuple(const ClassName & val) \
        { \
            return std::tie(val.m1, val.m2, val.m3); \
        } \
        template <> \
        inline auto object_as_tuple(ClassName & val) \
        { \
            return std::tie(val.m1, val.m2, val.m3); \
        } \
    }

#define AWL_SERIALIZABLE_CLASS_4(ClassName, m1, m2, m3, m4) \
    namespace awl \
    { \
        template <> \
        inline auto object_as_tuple(const ClassName & val) \
        { \
            return std::tie(val.m1, val.m2, val.m3, m4); \
        } \
        template <> \
        inline auto object_as_tuple(ClassName & val) \
        { \
            return std::tie(val.m1, val.m2, val.m3, m4); \
        } \
    }

#define AWL_SERIALIZABLE_CLASS_5(ClassName, m1, m2, m3, m4, m5) \
    namespace awl \
    { \
        template <> \
        inline auto object_as_tuple(const ClassName & val) \
        { \
            return std::tie(val.m1, val.m2, val.m3, m4, m5); \
        } \
        template <> \
        inline auto object_as_tuple(ClassName & val) \
        { \
            return std::tie(val.m1, val.m2, val.m3, m4, m5); \
        } \
    }

#define AWL_BINARY_OPERATOR(ClassName, OP) \
    inline bool operator OP (const ClassName & left, const ClassName & right) \
    { \
        return awl::object_as_tuple(left) OP awl::object_as_tuple(right); \
    }

#define AWL_EQUATABLE(ClassName) \
    AWL_BINARY_OPERATOR(ClassName, ==) \
    AWL_BINARY_OPERATOR(ClassName, !=)

#define AWL_COMPARABLE(ClassName) \
    AWL_BINARY_OPERATOR(ClassName, <) \
    AWL_BINARY_OPERATOR(ClassName, >) \
    AWL_BINARY_OPERATOR(ClassName, <=) \
    AWL_BINARY_OPERATOR(ClassName, >=)

#define AWL_EQUATABLE_AND_COMPARABLE(ClassName) \
    AWL_EQUATABLE(ClassName) \
    AWL_COMPARABLE(ClassName)
