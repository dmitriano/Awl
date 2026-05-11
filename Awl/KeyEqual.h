/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <functional>
#include <type_traits>

namespace awl
{
    template <class T, auto get_key>
    class KeyEqual
    {
    private:

        using Key = std::remove_cvref_t<std::invoke_result_t<decltype(get_key), const T&>>;

    public:

        using is_transparent = void;

        bool operator()(const T& left, const T& right) const
        {
            return std::invoke(get_key, left) == std::invoke(get_key, right);
        }

        bool operator()(const T& left, const Key& right) const
        {
            return std::invoke(get_key, left) == right;
        }

        bool operator()(const Key& left, const T& right) const
        {
            return left == std::invoke(get_key, right);
        }

        bool operator()(const Key& left, const Key& right) const
        {
            return left == right;
        }
    };
}
