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
        { p.template tryGet<int>(std::declval<const char*>(), std::declval<int&>()) } -> std::same_as<bool>;
        { p.template tryGet<std::string>(std::declval<const char*>(), std::declval<std::string&>()) } -> std::same_as<bool>;
        { p.template tryGet<std::wstring>(std::declval<const char*>(), std::declval<std::wstring&>()) } -> std::same_as<bool>;

        { p.template set<int>(std::declval<const char*>(), std::declval<const int&>()) } -> std::same_as<void>;
        { p.template set<std::string>(std::declval<const char*>(), std::declval<const std::string&>()) } -> std::same_as<void>;
        { p.template set<std::wstring>(std::declval<const char*>(), std::declval<const std::wstring&>()) } -> std::same_as<void>;

        { p.clear() } -> std::same_as<void>;
    };

    // For acessing Command Line attributes without creating TestContext.
    template <attribute_provider Provider>
    struct ProviderContext
    {
        Provider& attributeProvider;
    };

    template <class T, attribute_provider Provider>
    T getAttributeValue(Provider& provider, const char* name, T default_val)
    {
        T val;

        if (!provider.tryGet(name, val))
        {
            // Add default values to JSON.
            provider.set(name, default_val);

            return default_val;
        }

        return val;
    }
}
