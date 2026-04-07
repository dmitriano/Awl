/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

namespace awl
{
    template <class T>
    struct always_true
    {
        constexpr bool operator()(const T&) const noexcept
        {
            return true;
        }
    };

    template <class T>
    struct always_false
    {
        constexpr bool operator()(const T&) const noexcept
        {
            return false;
        }
    };
}
