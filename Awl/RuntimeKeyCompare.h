/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/KeyCompare.h"

#include <concepts>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>

namespace awl
{
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
    class RuntimeKeyCompare
    {
    public:

        using element_type = key_compare_element_t<T>;
        using value_type = T;
        using compare_value_type = key_compare_value_t<T>;
        using key_type = std::invoke_result_t<GetKey, const element_type&>;

        RuntimeKeyCompare() = default;

        constexpr RuntimeKeyCompare(GetKey get_key, Compare comp = {}) :
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

    template <class T, class Member, class Compare = std::less<void>>
    constexpr auto make_compare(Member T::* p, Compare comp = {})
    {
        using GetKey = member_getter<T, Member>;
        return RuntimeKeyCompare<T, GetKey, std::remove_const_t<std::decay_t<Compare>>>(GetKey{ p }, comp);
    }

    template <class T, class Member, class Compare = std::less<void>>
    constexpr auto make_shared_compare(Member T::* p, Compare comp = {})
    {
        using GetKey = member_getter<T, Member>;
        return RuntimeKeyCompare<std::shared_ptr<T>, GetKey, std::remove_const_t<std::decay_t<Compare>>>(GetKey{ p }, comp);
    }
}
