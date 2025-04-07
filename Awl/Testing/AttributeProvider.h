/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>

namespace awl::testing
{
    // We can't do this
    //
    // template <class P>
    // concept attribute_provider = requires(P& p)
    // {
    //     { template <class T> p.TryGet(std::declval<const char*>(), std::declval<T&>()) } -> std::same_as<bool>;
    // };

    // But we can check particular types at least.
    template <class P>
    concept attribute_provider = requires(P& p)
    {
        { p.template TryGet<int>(std::declval<const char*>(), std::declval<int&>()) } -> std::same_as<bool>;
        { p.template TryGet<std::string>(std::declval<const char*>(), std::declval<std::string&>()) } -> std::same_as<bool>;
        { p.template TryGet<std::wstring>(std::declval<const char*>(), std::declval<std::wstring&>()) } -> std::same_as<bool>;

        { p.template Set<int>(std::declval<const char*>(), std::declval<const int&>()) } -> std::same_as<void>;
        { p.template Set<std::string>(std::declval<const char*>(), std::declval<const std::string&>()) } -> std::same_as<void>;
        { p.template Set<std::wstring>(std::declval<const char*>(), std::declval<const std::wstring&>()) } -> std::same_as<void>;
    };

    // For acessing Command Line attributes without creating TestContext.
    template <attribute_provider Provider>
    struct ProviderContext
    {
        Provider& ap;
    };

    template <class T, attribute_provider Provider>
    T GetAttributeValue(Provider& provider, const char* name, T default_val)
    {
        T val;

        if (!provider.TryGet(name, val))
        {
            // Add default values to JSON.
            provider.Set(name, default_val);

            return default_val;
        }

        return val;
    }
}
