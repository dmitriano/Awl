/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Testing/AttributeProvider.h"
#include "Awl/TupleHelpers.h"

#include <tuple>
namespace awl::testing
{
    template <attribute_provider... Ps>
    class CompositeProvider
    {
    public:

        //Can be used if all the providers are default constructible.
        CompositeProvider() = default;

        //A template parameter pack cannot have a default argument.
        constexpr CompositeProvider(Ps... providers) : _providers(std::move(providers) ...) {}

        template <class T>
        bool tryGet(const char* name, T& val)
        {
            return tryGetAt<T, 0u>(name, val);
        }

        template <class T>
        void set(const char* name, const T& val)
        {
            std::apply([name, &val](Ps&... provider)
            {
                (provider.set(name, val), ...);

            }, _providers);
        }

        void clear()
        {
            std::apply([](Ps&... provider)
                {
                    (provider.clear(), ...);

                }, _providers);
        }

        template<std::size_t I>
        auto& get_provider() const
        {
            return std::get<I>(_providers);
        }

    private:

        using Tuple = std::tuple<std::decay_t<Ps>...>;

        template <class T, std::size_t Index>
        bool tryGetAt(const char* name, T& val)
        {
            if constexpr (Index == std::tuple_size_v<Tuple>)
            {
                static_cast<void>(name);
                static_cast<void>(val);
                return false;
            }
            else
            {
                auto& provider = std::get<Index>(_providers);

                if (provider.tryGet(name, val))
                {
                    return true;
                }

                return tryGetAt<T, Index + 1>(name, val);
            }
        }

        Tuple _providers;
    };

    template <class ... Ps>
    constexpr CompositeProvider<Ps ...> compose_providers(Ps... providers)
    {
        return CompositeProvider<std::remove_const_t<std::decay_t<Ps>>...>(std::move(providers) ...);
    }
}
