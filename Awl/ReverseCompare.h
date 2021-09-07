#pragma once

namespace awl
{
    template <class T, class Compare>
    class ReverseCompare
    {
    public:

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
    ReverseCompare<T, Compare> reverse_comparer(Compare comp)
    {
        return ReverseCompare<T, Compare>(std::move(comp));
    }
}
