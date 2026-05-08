/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

namespace awl
{
    template <class T, class Compare>
    class ReverseCompare
    {
    public:

        //The same as Container::value_type, for example std::shared_ptr<A>.
        using value_type = T;

        constexpr ReverseCompare(Compare comp = Compare()) : _comp(std::move(comp))
        {}

        constexpr bool operator()(const T& left, const T& right) const
        {
            return _comp(right, left);
        }

    private:

        Compare _comp;
    };

    template <class T, class Compare>
    class TransparentReverseCompare
    {
    public:

        //The same as Container::value_type, for example std::shared_ptr<A>.
        using value_type = T;

        //The type of the key for heterogeneous lookup.
        using key_type = typename Compare::key_type;

        constexpr TransparentReverseCompare(Compare comp = Compare()) : _comp(std::move(comp))
        {}

        constexpr bool operator()(const T& left, const T& right) const
        {
            return _comp(right, left);
        }

        constexpr bool operator()(const T& val, const key_type& id) const
        {
            return _comp(id, val);
        }

        constexpr bool operator()(const key_type& id, const T& val) const
        {
            return _comp(val, id);
        }

        using is_transparent = void;

    private:

        Compare _comp;
    };

    template <class T, class Compare>
    constexpr ReverseCompare<T, Compare> reverseComparer(Compare comp)
    {
        return ReverseCompare<T, std::remove_const_t<std::decay_t<Compare>>>(std::move(comp));
    }

    template <class T, class Compare>
    constexpr TransparentReverseCompare<T, Compare> reverseTransparentComparer(Compare comp)
    {
        return TransparentReverseCompare<T, std::remove_const_t<std::decay_t<Compare>>>(std::move(comp));
    }
}
