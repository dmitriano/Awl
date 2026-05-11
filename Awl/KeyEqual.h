/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <memory>
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
            return _getKey(left) == _getKey(right);
        }

        bool operator()(const T& left, const Key& right) const
        {
            return _getKey(left) == right;
        }

        bool operator()(const Key& left, const T& right) const
        {
            return left == _getKey(right);
        }

        bool operator()(const Key& left, const Key& right) const
        {
            return left == right;
        }

    private:

        GetKey _getKey;
    };

    template <class T, class GetKey>
    class KeyEqual<std::shared_ptr<T>, GetKey>
    {
    private:

        using Key = std::remove_cvref_t<typename GetKey::value_type>;

    public:

        using is_transparent = void;

        bool operator()(const std::shared_ptr<T>& left, const std::shared_ptr<T>& right) const
        {
            return _getKey(*left) == _getKey(*right);
        }

        bool operator()(const std::shared_ptr<T>& left, const Key& right) const
        {
            return _getKey(*left) == right;
        }

        bool operator()(const Key& left, const std::shared_ptr<T>& right) const
        {
            return left == _getKey(*right);
        }

        bool operator()(const Key& left, const Key& right) const
        {
            return left == right;
        }

    private:

        GetKey _getKey;
    };
}
