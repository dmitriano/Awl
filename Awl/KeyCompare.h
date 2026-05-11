/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Tuplizable.h"
#include "Awl/TypeTraits.h"

#include <concepts>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>

namespace awl
{
    template <class T>
    struct key_compare_element
    {
        using type = T;
    };

    template <class T>
    struct key_compare_element<T *>
    {
        using type = std::remove_cv_t<T>;
    };

    template <class T>
    struct key_compare_element<std::shared_ptr<T>>
    {
        using type = T;
    };

    template <class T, class Deleter>
    struct key_compare_element<std::unique_ptr<T, Deleter>>
    {
        using type = T;
    };

    template <class T>
    using key_compare_element_t = typename key_compare_element<T>::type;

    template <class T>
    struct key_compare_value
    {
        using type = T;
    };

    template <class T>
    struct key_compare_value<T *>
    {
        using type = const std::remove_cv_t<T>*;
    };

    template <class T>
    using key_compare_value_t = typename key_compare_value<T>::type;

    template <class T, class Member>
    class member_getter
    {
    public:

        using object_type = T;
        using value_type = std::invoke_result_t<Member T::*, const T&>;

        constexpr member_getter(Member T::* p) :
            _p(p)
        {}

        constexpr decltype(auto) operator()(const T& val) const
        {
            return std::invoke(_p, val);
        }

    private:

        Member T::* _p;
    };

    template <class T, class GetKey, class Compare = std::less<void>>
    class runtime_key_compare
    {
    public:

        using element_type = key_compare_element_t<T>;
        using value_type = T;
        using compare_value_type = key_compare_value_t<T>;
        using key_type = std::invoke_result_t<GetKey, const element_type&>;

        runtime_key_compare() = default;

        constexpr runtime_key_compare(GetKey get_key, Compare comp = {}) :
            getKey(std::move(get_key)),
            _comp(std::move(comp))
        {}

        constexpr bool operator()(const compare_value_type& left, const compare_value_type& right) const
        {
            return _comp(project(left), project(right));
        }

        constexpr bool operator()(const compare_value_type& val, const key_type & id) const
        {
            return _comp(project(val), id);
        }

        constexpr bool operator()(const key_type & id, const compare_value_type& val) const
        {
            return _comp(id, project(val));
        }

        using is_transparent = void;

    private:

        constexpr decltype(auto) project(const compare_value_type& val) const
        {
            if constexpr (std::invocable<GetKey, const compare_value_type&>)
            {
                return std::invoke(getKey, val);
            }
            else
            {
                return std::invoke(getKey, *val);
            }
        }

        [[no_unique_address]] GetKey getKey;
        [[no_unique_address]] Compare _comp;
    };

    template <class T, auto get_key, class Compare = std::less<void>>
    class KeyCompare
    {
    public:

        using element_type = key_compare_element_t<T>;
        using value_type = T;
        using compare_value_type = key_compare_value_t<T>;
        using key_type = std::invoke_result_t<decltype(get_key), const element_type&>;

        KeyCompare() = default;

        constexpr KeyCompare(Compare comp) :
            _comp(std::move(comp))
        {}

        constexpr bool operator()(const compare_value_type& left, const compare_value_type& right) const
        {
            return _comp(project(left), project(right));
        }

        constexpr bool operator()(const compare_value_type& val, const key_type & id) const
        {
            return _comp(project(val), id);
        }

        constexpr bool operator()(const key_type & id, const compare_value_type& val) const
        {
            return _comp(id, project(val));
        }

        using is_transparent = void;

    private:

        static constexpr decltype(auto) project(const compare_value_type& val)
        {
            if constexpr (std::invocable<decltype(get_key), const compare_value_type&>)
            {
                return std::invoke(get_key, val);
            }
            else
            {
                return std::invoke(get_key, *val);
            }
        }

        [[no_unique_address]] Compare _comp;
    };

    template <class T, class Member, class Compare = std::less<void>>
    constexpr auto make_compare(Member T::* p, Compare comp = {})
    {
        using GetKey = member_getter<T, Member>;
        return runtime_key_compare<T, GetKey, std::remove_const_t<std::decay_t<Compare>>>(GetKey{ p }, comp);
    }

    template <class T, class Member, class Compare = std::less<void>>
    constexpr auto make_shared_compare(Member T::* p, Compare comp = {})
    {
        using GetKey = member_getter<T, Member>;
        return runtime_key_compare<std::shared_ptr<T>, GetKey, std::remove_const_t<std::decay_t<Compare>>>(GetKey{ p }, comp);
    }

    template <class T, size_t index>
    struct tuplizable_getter
    {
        constexpr decltype(auto) operator()(const T& val) const
        {
            return std::get<index>(object_as_const_tuple(val));
        }
    };

    template <class T, size_t index>
    using tuplizable_compare = KeyCompare<T, tuplizable_getter<remove_pointer_t<T>, index>{}>;

    template <class>
    struct member_pointer_traits;

    template <class T, class Member>
    struct member_pointer_traits<Member T::*>
    {
        using object_type = T;
    };

    template <auto value, class Compare = std::less<void>>
    using member_compare = KeyCompare<typename member_pointer_traits<decltype(value)>::object_type, value, Compare>;

    template <auto value, class Compare = std::less<void>>
    using pointer_compare = KeyCompare<typename member_pointer_traits<decltype(value)>::object_type*, value, Compare>;

    template <auto value, class Compare = std::less<void>>
    using shared_compare = KeyCompare<std::shared_ptr<typename member_pointer_traits<decltype(value)>::object_type>, value, Compare>;

    template <auto value, class Compare = std::less<void>>
    using unique_compare = KeyCompare<std::unique_ptr<typename member_pointer_traits<decltype(value)>::object_type>, value, Compare>;

    // When we have a pointer type declared as using ObjectPtr = std::shared_ptr<T>, for example.
    template <class T, auto value, class Compare = std::less<void>>
    using smart_compare = KeyCompare<T, value, Compare>;
}
