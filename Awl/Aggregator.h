#pragma once

#include <tuple>
#include <optional>
#include <functional>
#include <memory>

#include "Awl/TupleHelpers.h"

namespace awl
{
    //Calls a function when its arguments have been set.
    template <class... Ts>
    class aggregator
    {
    private:

        using OptionalTuple = std::tuple<std::optional<std::decay_t<Ts>>...>;
        using Tuple = std::tuple<std::decay_t<Ts>...>;
        using Func = std::function<void (Ts...)>;

    public:

        aggregator(Func func) : m_func(std::move(func)), m_cancelled(false)
        {
        }

        template <std::size_t i>
        void set(std::tuple_element_t<i, Tuple> val)
        {
            std::get<i>(m_values) = std::move(val);

            if (all() && !m_cancelled)
            {
                call(std::make_index_sequence<std::tuple_size_v<Tuple>>());
            }
        }
        
        constexpr bool all() const
        {
            bool found = false;

            for_each(m_values, [&found](const auto& opt)
            {
                if (!opt)
                {
                    found = true;
                }
            });

            return !found;
        }

        constexpr void cancel()
        {
            m_cancelled = true;
        }

    private:

        template <std::size_t... index>
        constexpr void call(std::index_sequence<index...>)
        {
            m_func(std::move(*std::get<index>(m_values))...);
        }

        OptionalTuple m_values;
        
        Func m_func;

        bool m_cancelled;
    };

    template <class... Ts>
    auto make_shared_aggregator(std::function<void(Ts...)> func)
    {
        using A = aggregator<Ts...>;

        return std::make_shared<A>(func);
    }
}
