#pragma once

#include "Awl/FunctionTraits.h"
#include "Awl/Serializable.h"

#include <type_traits>

namespace awl
{
    template <class T, class GetKey>
    class KeyCompare
    {
    public:

        using Key = typename awl::function_traits<GetKey>::result_type;

        KeyCompare() = default;
        
        KeyCompare(GetKey && get_key) : getKey(std::forward<GetKey>(get_key))
        {
        }

        constexpr bool operator()(const T& left, const T& right) const
        {
            return getKey(left) < getKey(right);
        }

        constexpr bool operator()(const T& val, const Key & id) const
        {
            return getKey(val) < id;
        }

        constexpr bool operator()(const Key & id, const T& val) const
        {
            return id < getKey(val);
        }

    private:

        GetKey getKey;
    };

    template <class T, class GetKey>
    class KeyCompare<T *, GetKey>
    {
    public:

        using Key = typename awl::function_traits<GetKey>::result_type;

        KeyCompare() = default;

        KeyCompare(GetKey && get_key) : getKey(std::forward<GetKey>(get_key))
        {
        }

        constexpr bool operator()(const T * left, const T * right) const
        {
            return getKey(*left) < getKey(*right);
        }

        constexpr bool operator()(const T * val, const Key & id) const
        {
            return getKey(*val) < id;
        }

        constexpr bool operator()(const Key & id, const T * val) const
        {
            return id < getKey(*val);
        }

    private:

        GetKey getKey;
    };

    template <class T, class Field, Field T::*field_ptr>
    struct FieldGeter
    {
        const Field & operator() (const T & val) const
        {
            return val.*field_ptr;
        }
    };
    
    template <class T, class Field, Field std::remove_pointer_t<T>::*field_ptr>
    using FieldCompare = KeyCompare<T, FieldGeter<std::remove_pointer_t<T>, Field, field_ptr>>;

    template <class T, class Field, const Field & (T::*func_ptr)() const>
    struct FuncGeter
    {
        const Field & operator() (const T & val) const
        {
            return (val.*func_ptr)();
        }
    };

    template <class T, class Field, const Field & (std::remove_pointer_t<T>::*func_ptr)() const>
    using FuncCompare = KeyCompare<T, FuncGeter<std::remove_pointer_t<T>, Field, func_ptr>>;

    template <class T, size_t index>
    struct TuplizableGeter
    {
        const auto & operator() (const T & val) const
        {
            return std::get<index>(val.as_const_tuple());
        }
    };

    template <class T, size_t index>
    using TuplizableCompare = KeyCompare<T, TuplizableGeter<std::remove_pointer_t<T>, index>>;
}
