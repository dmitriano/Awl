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
    template <class T, auto get_key, class Equals = std::equal_to<void>>
    class KeyEqual
    {
    private:

        using Key = std::remove_cvref_t<std::invoke_result_t<decltype(get_key), const T&>>;

    public:

        using is_transparent = void;

        KeyEqual() = default;

        constexpr KeyEqual(Equals equals) :
            _equals(std::move(equals))
        {}

        bool operator()(const T& left, const T& right) const
        {
            return _equals(std::invoke(get_key, left), std::invoke(get_key, right));
        }

        bool operator()(const T& left, const Key& right) const
        {
            return _equals(std::invoke(get_key, left), right);
        }

        bool operator()(const Key& left, const T& right) const
        {
            return _equals(left, std::invoke(get_key, right));
        }

        bool operator()(const Key& left, const Key& right) const
        {
            return _equals(left, right);
        }

    private:

        [[no_unique_address]] Equals _equals;
    };
}
