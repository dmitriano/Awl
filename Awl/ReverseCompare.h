#pragma once

namespace awl
{
    template <class T, class Compare>
    class ReverseCompare
    {
    public:

        ReverseCompare(Compare && comp = Compare()) : m_comp(std::forward<Compare>(comp))
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
    inline ReverseCompare<T, Compare> compose_comparers(Compare && comp)
    {
        return ReverseCompare<T, Compare>(std::forward<Compare>(comp) ...);
    }
}
