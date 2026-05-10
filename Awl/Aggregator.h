/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/TupleHelpers.h"

#include <tuple>
#include <optional>
#include <functional>
#include <memory>

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

        aggregator(Func func) : _func(std::move(func)), _cancelled(false)
        {}

        template <std::size_t i>
        void set(std::tuple_element_t<i, Tuple> val)
        {
            std::get<i>(_values) = std::move(val);

            if (all() && !_cancelled)
            {
                call(std::make_index_sequence<std::tuple_size_v<Tuple>>());
            }
        }
        
        constexpr bool all() const
        {
            bool found = false;

            for_each(_values, [&found](const auto& opt)
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
            _cancelled = true;
        }

    private:

        template <std::size_t... index>
        constexpr void call(std::index_sequence<index...>)
        {
            _func(std::move(*std::get<index>(_values))...);
        }

        OptionalTuple _values;
        
        Func _func;

        bool _cancelled;
    };

    template <class... Ts>
    auto make_shared_aggregator(std::function<void(Ts...)> func)
    {
        using A = aggregator<Ts...>;

        return std::make_shared<A>(func);
    }
}
