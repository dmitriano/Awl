/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <cstddef>
#include <functional>
#include <type_traits>
#include <utility>

namespace awl
{
    template <class T, auto get_key,
        class Hash = std::hash<std::remove_cvref_t<std::invoke_result_t<decltype(get_key), const T&>>>>
    class KeyHash
    {
    private:

        using Key = std::remove_cvref_t<std::invoke_result_t<decltype(get_key), const T&>>;

    public:

        using is_transparent = void;

        KeyHash() = default;

        constexpr KeyHash(Hash hash) :
            _hash(std::move(hash))
        {}

        std::size_t operator()(const T& val) const
        {
            return _hash(std::invoke(get_key, val));
        }

        std::size_t operator()(const Key& key) const
        {
            return _hash(key);
        }

    private:

        [[no_unique_address]] Hash _hash;
    };
}
