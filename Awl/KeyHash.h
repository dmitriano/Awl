/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <type_traits>

namespace awl
{
    template <class T, class GetKey>
    class KeyHash
    {
    private:

        using Key = std::remove_cvref_t<std::invoke_result_t<GetKey, const T&>>;

    public:

        using is_transparent = void;

        std::size_t operator()(const T& val) const
        {
            return std::hash<Key>{}(std::invoke(_getKey, val));
        }

        std::size_t operator()(const Key& key) const
        {
            return std::hash<Key>{}(key);
        }

    private:

        GetKey _getKey;
    };

    template <class T, class GetKey>
    class KeyHash<std::shared_ptr<T>, GetKey>
    {
    private:

        using Key = std::remove_cvref_t<std::invoke_result_t<GetKey, const T&>>;

    public:

        using is_transparent = void;

        std::size_t operator()(const std::shared_ptr<T>& val) const
        {
            return std::hash<Key>{}(std::invoke(_getKey, *val));
        }

        std::size_t operator()(const Key& key) const
        {
            return std::hash<Key>{}(key);
        }

    private:

        GetKey _getKey;
    };
}
