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
    constexpr auto make_compare(Field T::* p, Compare comp = {})
    {
        return KeyCompare<T, field_getter<T, Field>, std::remove_const_t<std::decay_t<Compare>>>(p, comp);
    }

    template <class T, class ReturnType, class Compare = std::less<void>>
    constexpr auto make_compare(FuncPtr<T, ReturnType> p, Compare comp = {})
    {
        return KeyCompare<T, func_getter<T, ReturnType>, std::remove_const_t<std::decay_t<Compare>>>(p, comp);
    }

    template <class T, class ReturnType, class Compare = std::less<void>>
    constexpr auto make_shared_compare(FuncPtr<T, ReturnType> p, Compare comp = {})
    {
        return KeyCompare<std::shared_ptr<T>, func_getter<T, ReturnType>, std::remove_const_t<std::decay_t<Compare>>>(p, comp);
    }

    template <class T, size_t index>
    using tuplizable_compare = KeyCompare<T, tuplizable_getter<remove_pointer_t<T>, index>>;

    template <auto value, class Compare = std::less<void>>
    using member_compare = KeyCompare<typename getter<value>::object_type, getter<value>, Compare>;

    template <auto value, class Compare = std::less<void>>
    using pointer_compare = KeyCompare<typename getter<value>::object_type*, getter<value>, Compare>;

    template <auto value, class Compare = std::less<void>>
    using shared_compare = KeyCompare<std::shared_ptr<typename getter<value>::object_type>, getter<value>, Compare>;

    template <auto value, class Compare = std::less<void>>
    using unique_compare = KeyCompare<std::unique_ptr<typename getter<value>::object_type>, getter<value>, Compare>;

    // When we have a pointer type declared as using ObjectPtr = std::shared_ptr<T>, for example.
    template <class T, auto value, class Compare = std::less<void>>
    using smart_compare = KeyCompare<T, getter<value>, Compare>;

    // For using with std::ranges::filter without std::bind.
    template <class Field, class Proj>
    class projected_equal_to
    {
    public:

        projected_equal_to(Field field, Proj proj) :
            m_proj(proj),
            m_field(std::move(field))
        {}

        // object_val can be of type std::shared_ptr, for example.
        bool operator() (const auto& object_val) const
        {
            return std::invoke(m_proj, object_val) == m_field;
        }

    private:

        Proj m_proj;
        Field m_field;
    };

    template <class Value, class Field>
    auto mem_fn_equal_to(Field(Value::* field_ptr)() const, std::decay_t<Field> field)
    {
        return projected_equal_to<Field, decltype(field_ptr)>(field, field_ptr);
    }

    template <class Value, class Field>
    auto mem_fn_equal_to(Field Value::*field_ptr, Field field)
    {
        return projected_equal_to<Field, decltype(field_ptr)>(field, field_ptr);
    }
}
