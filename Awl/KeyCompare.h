#pragma once

#include "Awl/FunctionTraits.h"

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

    template <class T, class Key, Key T::*field_ptr>
    class FieldCompare
    {
    public:

        constexpr bool operator()(const T& left, const T& right) const
        {
            return left.*field_ptr < right.*field_ptr;
        }

        constexpr bool operator()(const T& val, const Key & id) const
        {
            return val.*field_ptr < id;
        }

        constexpr bool operator()(const Key & id, const T& val) const
        {
            return id < val.*field_ptr;
        }
    };

    template <class T, class Key, Key(T::*func_ptr)() const>
    class FuncCompare
    {
    public:

        constexpr bool operator()(const T& left, const T& right) const
        {
            return (left.*func_ptr)() < (right.*func_ptr)();
        }

        constexpr bool operator()(const T& val, const Key & id) const
        {
            return (val.*func_ptr)() < id;
        }

        constexpr bool operator()(const Key & id, const T& val) const
        {
            return id < (val.*func_ptr)();
        }
    };
}
