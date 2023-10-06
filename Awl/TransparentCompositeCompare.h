/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <tuple>

#include "Awl/TupleHelpers.h"
#include "Awl/CompositeCompare.h"

namespace awl
{
    // Requires the composing comparers to have key_type.
    template <class T, class ... Cs>
    class TransparentCompositeCompare
    {
    private:

        using InternalCompositeCompare = CompositeCompare<T, Cs...>;

    public:

        using value_type = T;

        // The tuple of const references.
        using key_type = std::tuple<const typename std::decay_t<Cs>::key_type&...>;

        // Can be used if all the comparers are default constructible.
        TransparentCompositeCompare() = default;
        
        // A template parameter pack cannot have a default argument.
        TransparentCompositeCompare(Cs... comp) : compositeComare(std::move(comp) ...) {}

        // Makes a mixed key for heterogeneous lookup from both references and values.
        template <typename... Args>
        static constexpr auto make_key(Args&&... args)
        {
            // The same elements as in key_type type but without const.
            using variable_key_type = std::tuple<std::remove_cv_t<std::decay_t<typename std::decay_t<Cs>::key_type>>&...>;

            auto key = make_similar_tuple<variable_key_type>(std::forward<Args>(args)...);

            // Ensure we can pass key as the comparer argument.
            static_assert(std::is_convertible_v<decltype(key), key_type>);
            
            return key;
        }

        constexpr bool operator()(const T& left, const T& right) const
        {
            return compositeComare(left, right);
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

        using Tuple = typename InternalCompositeCompare::Tuple;

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
                auto& comp = std::get<Index>(compositeComare.m_comps);

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
                auto& comp = std::get<Index>(compositeComare.m_comps);

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

        InternalCompositeCompare compositeComare;
    };

    template <class T, class ... Cs>
    TransparentCompositeCompare<T, Cs ...> compose_transparent_comparers(Cs... comp)
    {
        return TransparentCompositeCompare<T, Cs ...>(std::move(comp) ...);
    }
}
