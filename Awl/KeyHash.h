/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <functional>

namespace awl
{
    template <auto member>
    class KeyHash;

    template <class T, class Key, Key T::* member>
    class KeyHash<member>
    {
    public:
        size_t operator()(const T& val) const
        {
            return std::hash<Key>{}(val.*member);
        }
    };

    template <auto member>
    class KeyEqual;

    template <class T, class Key, Key T::* member>
    class KeyEqual<member>
    {
    public:
        bool operator()(const T& left, const T& right) const
        {
            return left.*member == right.*member;
        }
    };
}
