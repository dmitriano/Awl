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

        ReverseCompare(Compare comp = Compare()) : m_comp(std::move(comp))
        {
        }

        constexpr bool operator()(const T& left, const T& right) const
        {
            return m_comp(right, left);
        }

    private:

        Compare m_comp;
    };

    template <class T, class Compare>
    class TransparentReverseCompare
    {
    public:

        //The same as Container::value_type, for example std::shared_ptr<A>.
        using value_type = T;

        //The type of the key for heterogeneous lookup.
        using key_type = typename Compare::key_type;

        TransparentReverseCompare(Compare comp = Compare()) : m_comp(std::move(comp))
        {
        }

        constexpr bool operator()(const T& left, const T& right) const
        {
            return m_comp(right, left);
        }

        constexpr bool operator()(const T& val, const key_type& id) const
        {
            return m_comp(id, val);
        }

        constexpr bool operator()(const key_type& id, const T& val) const
        {
            return m_comp(val, id);
        }

        using is_transparent = void;

    private:

        Compare m_comp;
    };

    template <class T, class Compare>
    ReverseCompare<T, Compare> reverse_comparer(Compare comp)
    {
        return ReverseCompare<T, Compare>(std::move(comp));
    }

    template <class T, class Compare>
    TransparentReverseCompare<T, Compare> reverse_transparent_comparer(Compare comp)
    {
        return TransparentReverseCompare<T, Compare>(std::move(comp));
    }
}
