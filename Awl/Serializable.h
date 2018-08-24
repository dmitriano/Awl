#pragma once

#include "Awl/TupleHelpers.h"

namespace awl
{
    template <class T>
    auto class_as_tuple(T & val)
    {
        return val.as_tuple();
    }

    template <class T>
    auto class_as_const_tuple(const T & val)
    {
        return val.as_tuple();
    }

    template <class T>
    bool objects_equal(const T & left, const T & right)
    {
        return class_as_const_tuple(left) == class_as_const_tuple(right);
    }

    template <class T>
    bool objects_less(const T & left, const T & right)
    {
        return class_as_const_tuple(left) < class_as_const_tuple(right);
    }
}

//Used inside of a class definition for defining as_tuple() member functions.
#define AWL_SERIALIZABLE(...) \
    auto as_tuple() const \
    { \
        return std::tie(__VA_ARGS__); \
    } \
    auto as_tuple() \
    { \
        return std::tie(__VA_ARGS__); \
    }

//Used outside of a class definition for defining class_as_tuple and class_as_const_tuple functions.
//It will become easy to do with __VA_OPT__ in C++20 but I don't see how to do this currently.
//Is not clear how to prepend VA_ARGS with 'val.' in std::tie() call.
//#define AWL_SERIALIZABLE_CLASS(ClassName, ...) \
    namespace awl \
    { \
        template <> \
        inline auto class_as_tuple(const ClassName & val) \
        { \
            return std::tie(__VA_ARGS__); \
        } \
        template <> \
        inline auto class_as_tuple(ClassName & val) \
        { \
            return std::tie(__VA_ARGS__); \
        } \
    }

#define AWL_BINARY_OPERATOR(ClassName, OP) \
    inline bool operator OP (const ClassName & left, const ClassName & right) \
    { \
        return awl::class_as_const_tuple(left) OP awl::class_as_const_tuple(right); \
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
