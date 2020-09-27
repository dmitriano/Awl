#pragma once

#include <tuple>

namespace awl
{
    template <class T, class ... Cs>
    class CompositeCompare
    {
    public:

        CompositeCompare(Cs && ... comp) : m_comps(std::forward<Cs>(comp) ...)
        {
        }

        constexpr bool operator()(const T& left, const T& right) const
        {
            return Compare<0u>(left, right);
        }

    private:

        using Tuple = std::tuple<Cs ...>;

        template <size_t Index>
        bool Compare(const T& left, const T& right) const
        {
            auto & comp = std::get<Index>(m_comps);

            if (comp(left, right))
            {
                return true;
            }

            if (comp(right, left))
            {
                return false;
            }

            return Compare<Index + 1>(left, right);
        }

        template<>
        bool Compare<std::tuple_size_v<Tuple>>(const T&, const T&) const
        {
            return false;
        }

        Tuple m_comps;
    };

    template <class T, class ... Cs>
    inline auto compose_comparers(Cs && ... comp)
    {
        return CompositeCompare<T, Cs ...>(std::forward<Cs>(comp) ...);
    }
}
