/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/FunctionTraits.h"
#include "Awl/Tuplizable.h"
#include "Awl/TypeTraits.h"
#include "Awl/Getters.h"

#include <type_traits>
#include <memory>

namespace awl
{
    template <class T, class GetKey, class Compare = std::less<void>>
    class KeyCompare
    {
    public:

        //The type GetKey applied to, for example 'class A { int key; std::string other; };'.
        using element_type = T;

        //The same as Container::value_type, for example std::shared_ptr<A>.
        using value_type = T;

        //The type of the key for heterogeneous lookup.
        using key_type = typename awl::function_traits<GetKey>::result_type;

        KeyCompare() = default;
        
        constexpr KeyCompare(GetKey get_key, Compare comp = {}) :
            getKey(std::move(get_key)),
            m_comp(std::move(comp))
        {}

        //KeyCompare(const KeyCompare&) = default;
        //KeyCompare(KeyCompare&&) = default;
        //KeyCompare& operator = (const KeyCompare&) = default;
        //KeyCompare& operator = (KeyCompare&&) = default;

        constexpr bool operator()(const T& left, const T& right) const
        {
            return m_comp(getKey(left), getKey(right));
        }

        constexpr bool operator()(const T& val, const key_type & id) const
        {
            return m_comp(getKey(val), id);
        }

        constexpr bool operator()(const key_type & id, const T& val) const
        {
            return m_comp(id, getKey(val));
        }

        using is_transparent = void;

    private:

        GetKey getKey;

        Compare m_comp;
    };

    template <class T, class GetKey, class Compare>
    class KeyCompare<T *, GetKey, Compare>
    {
    public:

        using value_type = T *;
        using element_type = T;
        using key_type = typename awl::function_traits<GetKey>::result_type;

        KeyCompare() = default;

        constexpr KeyCompare(GetKey get_key, Compare comp = {}) :
            getKey(std::move(get_key)),
            m_comp(std::move(comp))
        {}

        //KeyCompare(const KeyCompare&) = default;
        //KeyCompare(KeyCompare&&) = default;
        //KeyCompare& operator = (const KeyCompare&) = default;
        //KeyCompare& operator = (KeyCompare&&) = default;

        constexpr bool operator()(const T * left, const T * right) const
        {
            return m_comp(getKey(*left), getKey(*right));
        }

        constexpr bool operator()(const T * val, const key_type & id) const
        {
            return m_comp(getKey(*val), id);
        }

        constexpr bool operator()(const key_type & id, const T * val) const
        {
            return m_comp(id, getKey(*val));
        }

    private:

        GetKey getKey;

        Compare m_comp;
    };

    template <class T, class GetKey, class Compare>
    class KeyCompare<std::shared_ptr<T>, GetKey, Compare>
    {
    public:

        using value_type = std::shared_ptr<T>;
        using element_type = T;
        using key_type = typename awl::function_traits<GetKey>::result_type;

        KeyCompare() = default;

        constexpr KeyCompare(GetKey get_key, Compare comp = {}) :
            getKey(std::move(get_key)),
            m_comp(std::move(comp))
        {}

        //KeyCompare(const KeyCompare&) = default;
        //KeyCompare(KeyCompare&&) = default;
        //KeyCompare& operator = (const KeyCompare&) = default;
        //KeyCompare& operator = (KeyCompare&&) = default;

        constexpr bool operator()(const std::shared_ptr<T> & left, const std::shared_ptr<T> & right) const
        {
            return m_comp(getKey(*left), getKey(*right));
        }

        constexpr bool operator()(const std::shared_ptr<T> & val, const key_type & id) const
        {
            return m_comp(getKey(*val), id);
        }

        constexpr bool operator()(const key_type & id, const std::shared_ptr<T> & val) const
        {
            return m_comp(id, getKey(*val));
        }

    private:

        GetKey getKey;

        Compare m_comp;
    };

    template <class T, class Deleter, class GetKey, class Compare>
    class KeyCompare<std::unique_ptr<T, Deleter>, GetKey, Compare>
    {
    public:

        using value_type = std::unique_ptr<T, Deleter>;
        using element_type = T;
        using key_type = typename awl::function_traits<GetKey>::result_type;

        KeyCompare() = default;

        constexpr KeyCompare(GetKey get_key, Compare comp = {}) :
            getKey(std::move(get_key)),
            m_comp(std::move(comp))
        {}

        constexpr bool operator()(const std::unique_ptr<T, Deleter> & left, const std::unique_ptr<T, Deleter> & right) const
        {
            return m_comp(getKey(*left), getKey(*right));
        }

        constexpr bool operator()(const std::unique_ptr<T, Deleter> & val, const key_type & id) const
        {
            return m_comp(getKey(*val), id);
        }

        constexpr bool operator()(const key_type & id, const std::unique_ptr<T, Deleter> & val) const
        {
            return m_comp(id, getKey(*val));
        }

    private:

        GetKey getKey;

        Compare m_comp;
    };

    template <class T, class Field, class Compare = std::less<void>>
    using FieldCompare = KeyCompare<T, field_getter<remove_pointer_t<T>, Field>, Compare>;

    template <class T, class Field, class Compare = std::less<void>>
    constexpr auto make_field_compare(Field T::* p, Compare comp = {})
    {
        return FieldCompare<T, Field, std::remove_const_t<std::decay_t<Compare>>>(p, comp);
    }

    template <class T, class Field, class Compare = std::less<void>>
    using FuncCompare = KeyCompare<T, func_getter<remove_pointer_t<T>, Field>, Compare>;

    template <class T, class ReturnType, class Compare = std::less<void>>
    constexpr auto make_func_compare(FuncPtr<T, ReturnType> p, Compare comp = {})
    {
        return FuncCompare<T, ReturnType, std::remove_const_t<std::decay_t<Compare>>>(p, comp);
    }

    template <class T, class ReturnType, class Compare = std::less<void>>
    constexpr auto make_shared_func_compare(FuncPtr<T, ReturnType> p, Compare comp = {})
    {
        return FuncCompare<std::shared_ptr<T>, ReturnType, std::remove_const_t<std::decay_t<Compare>>>(p, comp);
    }

    template <class T, size_t index>
    struct TuplizableGetter
    {
        const auto & operator() (const T & val) const
        {
            return std::get<index>(val.as_const_tuple());
        }
    };

    template <class T, size_t index>
    using TuplizableCompare = KeyCompare<T, TuplizableGetter<remove_pointer_t<T>, index>>;
}
