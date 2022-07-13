/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <tuple>

namespace awl
{
    template <class T, class ... Cs>
    class CompositeCompare
    {
    public:

        using value_type = T;

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

        template <std::size_t Index>
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
                auto& comp = std::get<Index>(m_comps);

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
    class TransparentCompositeCompare
    {
    public:

        using value_type = T;

        //The type of the key for heterogeneous lookup.
        using key_type = std::tuple<typename std::decay_t<Cs>::key_type...>;

        //Can be used if all the comparers are default constructible.
        TransparentCompositeCompare() = default;
        
        //A template parameter pack cannot have a default argument.
        TransparentCompositeCompare(Cs... comp) : m_comps(std::move(comp) ...)
        {
        }

        constexpr bool operator()(const T& left, const T& right) const
        {
            return Compare<0u>(left, right);
        }

        constexpr bool operator()(const T& val, const key_type& id) const
        {
            return Compare<0u>(val, id);
        }

        constexpr bool operator()(const key_type& id, const T& val) const
        {
            return Compare<0u>(id, val);
        }

        using is_transparent = void;

    private:

        using Tuple = std::tuple<std::decay_t<Cs>...>;

        template <std::size_t Index>
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
                auto& comp = std::get<Index>(m_comps);

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

        template <std::size_t Index>
        bool Compare(const T& left, const key_type& right_key) const
        {
            if constexpr (Index == std::tuple_size_v<Tuple>)
            {
                static_cast<void>(left);
                static_cast<void>(right_key);
                return false;
            }
            else
            {
                auto& comp = std::get<Index>(m_comps);

                auto& right = std::get<Index>(right_key);

                if (comp(left, right))
                {
                    return true;
                }

                if (comp(right, left))
                {
                    return false;
                }

                return Compare<Index + 1>(left, right_key);
            }
        }

        template <std::size_t Index>
        bool Compare(const key_type& left_key, const T& right) const
        {
            if constexpr (Index == std::tuple_size_v<Tuple>)
            {
                static_cast<void>(left_key);
                static_cast<void>(right);
                return false;
            }
            else
            {
                auto& comp = std::get<Index>(m_comps);

                auto& left = std::get<Index>(left_key);

                if (comp(left, right))
                {
                    return true;
                }

                if (comp(right, left))
                {
                    return false;
                }

                return Compare<Index + 1>(left_key, right);
            }
        }

        Tuple m_comps;
    };

    template <class T, class ... Cs>
    CompositeCompare<T, Cs ...> compose_comparers(Cs... comp)
    {
        return CompositeCompare<T, Cs ...>(std::move(comp) ...);
    }

    template <class T, class ... Cs>
    TransparentCompositeCompare<T, Cs ...> compose_transparent_comparers(Cs... comp)
    {
        return TransparentCompositeCompare<T, Cs ...>(std::move(comp) ...);
    }
}
