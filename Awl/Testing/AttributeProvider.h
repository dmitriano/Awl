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
        { p.template TryGet(std::declval<const char*>(), std::declval<int&>()) } -> std::same_as<bool>;
        { p.template TryGet(std::declval<const char*>(), std::declval<std::string&>()) } -> std::same_as<bool>;
        { p.template TryGet(std::declval<const char*>(), std::declval<std::wstring&>()) } -> std::same_as<bool>;
    };
}
