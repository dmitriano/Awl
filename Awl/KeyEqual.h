/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <type_traits>

namespace awl
{
    template <class T, class GetKey>
    class KeyEqual
    {
    private:
        using Key = std::remove_cvref_t<typename GetKey::value_type>;

    public:
        using is_transparent = void;

        bool operator()(const T& left, const T& right) const
        {
            return m_getKey(left) == m_getKey(right);
        }

        bool operator()(const T& left, const Key& right) const
        {
            return m_getKey(left) == right;
        }

        bool operator()(const Key& left, const T& right) const
        {
            return left == m_getKey(right);
        }

        bool operator()(const Key& left, const Key& right) const
        {
            return left == right;
        }

    private:
        GetKey m_getKey;
    };
}
