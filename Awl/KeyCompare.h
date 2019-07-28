#pragma once

#include "Awl/FunctionTraits.h"
#include "Awl/Serializable.h"

namespace awl
{
    template <class T, class GetKey>
    class KeyCompare
    {
    public:

        using Key = typename awl::function_traits<GetKey>::result_type;

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
    class FieldCompare
    {
    public:

        using Key = Field;

        constexpr bool operator()(const T& left, const T& right) const
        {
            return left.*field_ptr < right.*field_ptr;
        }

        constexpr bool operator()(const T& val, const Field & id) const
        {
            return val.*field_ptr < id;
        }

        constexpr bool operator()(const Field & id, const T& val) const
        {
            return id < val.*field_ptr;
        }
    };

    template <class T, class Field, Field T::*field_ptr>
    class FieldCompare<T *, Field, field_ptr>
    {
    public:

        using Key = Field;

        constexpr bool operator()(const T * left, const T * right) const
        {
            return left->*field_ptr < right->*field_ptr;
        }

        constexpr bool operator()(const T * val, const Field & id) const
        {
            return val->*field_ptr < id;
        }

        constexpr bool operator()(const Field & id, const T * val) const
        {
            return id < val->*field_ptr;
        }
    };

    template <class T, class Field, Field (T::*func_ptr)() const>
    class FuncCompare
    {
    public:

        using Key = Field;

        constexpr bool operator()(const T& left, const T& right) const
        {
            return (left.*func_ptr)() < (right.*func_ptr)();
        }

        constexpr bool operator()(const T& val, const Field & id) const
        {
            return (val.*func_ptr)() < id;
        }

        constexpr bool operator()(const Field & id, const T& val) const
        {
            return id < (val.*func_ptr)();
        }
    };

    template <class T, class Field, Field (T::*func_ptr)() const>
    class FuncCompare<T *, Field, func_ptr>
    {
    public:

        using Key = Field;
        
        constexpr bool operator()(const T * left, const T * right) const
        {
            return (left->*func_ptr)() < (right->*func_ptr)();
        }

        constexpr bool operator()(const T * val, const Field & id) const
        {
            return (val->*func_ptr)() < id;
        }

        constexpr bool operator()(const Field & id, const T * val) const
        {
            return id < (val->*func_ptr)();
        }
    };

    template <class T, size_t index>
    class TuplizableCompare
    {
    public:

        using Key = std::remove_reference_t<std::tuple_element_t<index, typename tuplizable_traits<T>::Tie>>;

        constexpr bool operator()(const T& left, const T& right) const
        {
            return std::get<index>(left.as_const_tuple()) < std::get<index>(right.as_const_tuple());
        }

        constexpr bool operator()(const T& val, const Key & id) const
        {
            return std::get<index>(val.as_const_tuple()) < id;
        }

        constexpr bool operator()(const Key & id, const T& val) const
        {
            return id < std::get<index>(val.as_const_tuple());
        }
    };

    template <class T, size_t index>
    class TuplizableCompare<T *, index>
    {
    public:

        using Key = std::remove_reference_t<std::tuple_element_t<index, typename tuplizable_traits<T>::Tie>>;

        constexpr bool operator()(const T * left, const T * right) const
        {
            return std::get<index>(left->as_const_tuple()) < std::get<index>(right->as_const_tuple());
        }

        constexpr bool operator()(const T * val, const Key & id) const
        {
            return std::get<index>(val->as_const_tuple()) < id;
        }

        constexpr bool operator()(const Key & id, const T * val) const
        {
            return id < std::get<index>(val->as_const_tuple());
        }
    };
}
