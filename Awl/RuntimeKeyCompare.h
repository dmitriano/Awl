/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

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

        using key_type = std::remove_cvref_t<std::invoke_result_t<GetKey, const T&>>;

        RuntimeKeyCompare() = default;

        constexpr RuntimeKeyCompare(GetKey get_key, Compare comp = {}) :
            getKey(std::move(get_key)),
            _comp(std::move(comp))
        {}

        constexpr bool operator()(const T& left, const T& right) const
        {
            return _comp(std::invoke(getKey, left), std::invoke(getKey, right));
        }

        constexpr bool operator()(const T& val, const key_type & id) const
        {
            return _comp(std::invoke(getKey, val), id);
        }

        constexpr bool operator()(const key_type & id, const T& val) const
        {
            return _comp(id, std::invoke(getKey, val));
        }

        using is_transparent = void;

    private:

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
