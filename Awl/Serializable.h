#pragma once

#include "Awl/TupleHelpers.h"

#include <type_traits>

namespace awl
{
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

    template <class T, typename = void>
    struct is_tuplizable : std::false_type {};

    template <class T>
    struct is_tuplizable<T, std::void_t<decltype(T{}.as_const_tuple())>> : std::true_type {};

    template <class T>
    inline constexpr bool is_tuplizable_v = is_tuplizable<T>::value;

    template <class T>
    class tuplizable_traits
    {
    private:

        //decltype(object_as_tuple(T{})) produces a const Tie,
        //so we need to declare a variable of type T.
        struct Helper
        {
            static auto GetTie()
            {
                T val;
                return object_as_tuple(val);
            }
        };

    public:

        using ConstTie = decltype(object_as_const_tuple(T{}));
        using Tie = decltype(Helper::GetTie());
    };
}

//Used inside of a class definition for defining as_tuple() member functions.
//Theoretically all the as_tuple() overloads can be constexpr, but GCC 4.9 does not compile it.
#define AWL_SERIALIZABLE(...) \
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
