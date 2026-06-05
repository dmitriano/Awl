#pragma once

#include <typeinfo>

namespace awl::boost_json
{
    template <class T>
    constexpr auto type_hint()
    {
        return typeid(T).name();
    }
}
