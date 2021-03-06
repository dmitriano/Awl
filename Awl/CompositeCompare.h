#pragma once

#include <tuple>

namespace awl
{
    template <class T, class ... Cs>
    class CompositeCompare
    {
    public:

        //Can be used if all the comparers are default constructible.
        CompositeCompare() = default;
        
        //A template parameter pack cannot have a default argument.
        CompositeCompare(Cs... comp) : m_comps(std::move(comp) ...)
        {
        }

        constexpr bool operator()(const T& left, const T& right) const
        {
            return Compare<0u>(left, right);
        }

    private:

        using Tuple = std::tuple<std::decay_t<Cs>...>;

        template <size_t Index>
        bool Compare(const T& left, const T& right) const
        {
            if constexpr (Index == std::tuple_size_v<Tuple>)
            {
                static_cast<void>(left);
                static_cast<void>(right);
                return false;
            }
            else
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
        }

        Tuple m_comps;
    };

    template <class T, class ... Cs>
    inline CompositeCompare<T, Cs ...> compose_comparers(Cs... comp)
    {
        return CompositeCompare<T, Cs ...>(std::move(comp) ...);
    }
}
