/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <cstddef>
#include <functional>
#include <type_traits>

namespace awl
{
    template <class T, auto get_key>
    class KeyHash
    {
    private:

        using Key = std::remove_cvref_t<std::invoke_result_t<decltype(get_key), const T&>>;

    public:

        using is_transparent = void;

        std::size_t operator()(const T& val) const
        {
            return std::hash<Key>{}(std::invoke(get_key, val));
        }

        std::size_t operator()(const Key& key) const
        {
            return std::hash<Key>{}(key);
        }
    };
}
