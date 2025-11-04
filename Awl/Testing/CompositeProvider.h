/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <tuple>

#include "Awl/Testing/AttributeProvider.h"
#include "Awl/TupleHelpers.h"

namespace awl::testing
{
    template <attribute_provider... Ps>
    class CompositeProvider
    {
    public:

        //Can be used if all the providers are default constructible.
        CompositeProvider() = default;

        //A template parameter pack cannot have a default argument.
        constexpr CompositeProvider(Ps... providers) : m_providers(std::move(providers) ...) {}

        template <class T>
        bool TryGet(const char* name, T& val)
        {
            return TryGetAt<T, 0u>(name, val);
        }

        template <class T>
        void Set(const char* name, const T& val)
        {
            std::apply([name, &val](Ps&... provider)
            {
                (provider.Set(name, val), ...);

            }, m_providers);
        }

        void Clear()
        {
            std::apply([](Ps&... provider)
                {
                    (provider.Clear(), ...);

                }, m_providers);
        }

        template<std::size_t I>
        auto& get_provider() const
        {
            return std::get<I>(m_providers);
        }

    private:

        using Tuple = std::tuple<std::decay_t<Ps>...>;

        template <class T, std::size_t Index>
        bool TryGetAt(const char* name, T& val)
        {
            if constexpr (Index == std::tuple_size_v<Tuple>)
            {
                static_cast<void>(name);
                static_cast<void>(val);
                return false;
            }
            else
            {
                auto& provider = std::get<Index>(m_providers);

                if (provider.TryGet(name, val))
                {
                    return true;
                }

                return TryGetAt<T, Index + 1>(name, val);
            }
        }

        Tuple m_providers;
    };

    template <class ... Ps>
    constexpr CompositeProvider<Ps ...> compose_providers(Ps... providers)
    {
        return CompositeProvider<std::remove_const_t<std::decay_t<Ps>>...>(std::move(providers) ...);
    }
}
