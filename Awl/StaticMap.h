/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/String.h"
#include "Awl/StaticChain.h"
#include "Awl/StringFormat.h"

#include <map>
#include <stdexcept>

namespace awl
{
    template <class T>
    using StaticMap = std::map<const char*, const StaticLink<T>*, CStringLess<char>>;

    template <class T>
    StaticMap<T> make_static_map()
    {
        StaticMap<T> map;

        for (const StaticLink<T>* p_link : awl::static_chain<T>())
        {
            if (!map.emplace(p_link->name(), p_link).second)
            {
                throw std::runtime_error(aformat() << "The test '" << p_link->name() << " already exists.");
            }
        }

        return map;
    }
}
