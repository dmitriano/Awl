/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <functional>
#include <type_traits>
#include <utility>

namespace awl
{
    template <class T, auto get_key, class Compare = std::less<void>>
    class KeyCompare
    {
    public:

        using key_type = std::remove_cvref_t<std::invoke_result_t<decltype(get_key), const T&>>;

        KeyCompare() = default;

        constexpr KeyCompare(Compare comp) :
            _comp(std::move(comp))
        {}

        constexpr bool operator()(const T& left, const T& right) const
        {
            return _comp(std::invoke(get_key, left), std::invoke(get_key, right));
        }

        constexpr bool operator()(const T& val, const key_type & id) const
        {
            return _comp(std::invoke(get_key, val), id);
        }

        constexpr bool operator()(const key_type & id, const T& val) const
        {
            return _comp(id, std::invoke(get_key, val));
        }

        using is_transparent = void;

    private:

        [[no_unique_address]] Compare _comp;
    };
}
