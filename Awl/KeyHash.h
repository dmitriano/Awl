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
    template <class T, class GetKey>
    class KeyHash
    {
    private:

        using Key = std::remove_cvref_t<typename GetKey::value_type>;

    public:

        using is_transparent = void;

        std::size_t operator()(const T& val) const
        {
            return std::hash<Key>{}(m_getKey(val));
        }

        std::size_t operator()(const Key& key) const
        {
            return std::hash<Key>{}(key);
        }

    private:

        GetKey m_getKey;
    };
}
