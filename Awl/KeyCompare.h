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
    private:

        struct GetterCreator
        {
            static auto CreateGetter()
            {
                return [](const T & val) -> const Key { return val.*field_ptr; };
            }
        };

        using GetId = decltype(GetterCreator::CreateGetter());

    public:

        FieldCompare() : keyCompare(KeyCompare<T, GetId>(GetterCreator::CreateGetter()))
        {
        }

        constexpr bool operator()(const T& left, const T& right) const
        {
            return keyCompare(left, right);
        }

        constexpr bool operator()(const T& val, const Key & id) const
        {
            return keyCompare(val, id);
        }

        constexpr bool operator()(const Key & id, const T& val) const
        {
            return keyCompare(id, val);
        }

    private:

        KeyCompare<T, GetId> keyCompare;
    };

    template <class T, class Key, Key(T::*func_ptr)() const>
    class FuncCompare
    {
    private:

        struct GetterCreator
        {
            static auto CreateGetter()
            {
                return [](const T & val) -> Key { return (val.*func_ptr)(); };
            }
        };

        using GetId = decltype(GetterCreator::CreateGetter());

    public:

        FuncCompare() : keyCompare(KeyCompare<T, GetId>(GetterCreator::CreateGetter()))
        {
        }

        constexpr bool operator()(const T& left, const T& right) const
        {
            return keyCompare(left, right);
        }

        constexpr bool operator()(const T& val, const Key & id) const
        {
            return keyCompare(val, id);
        }

        constexpr bool operator()(const Key & id, const T& val) const
        {
            return keyCompare(id, val);
        }

    private:

        KeyCompare<T, GetId> keyCompare;
    };
}
