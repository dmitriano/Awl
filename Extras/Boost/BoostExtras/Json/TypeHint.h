#pragma once

#include <typeinfo>

namespace awl
{
    template <class T>
    constexpr auto type_hint()
    {
        return typeid(T).name();
    }
}
