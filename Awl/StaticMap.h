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
#include <regex>

namespace awl
{
    template <class T>
    using StaticMap = std::map<const char*, const StaticLink<T>*, CStringLess<char>>;

    namespace helpers
    {
        template <class T>
        void insert_static_link(StaticMap<T>& map, const StaticLink<T>* p_link)
        {
            if (!map.emplace(p_link->name(), p_link).second)
            {
                throw std::runtime_error(aformat() << "Static link '" << p_link->name() << " already exists.");
            }
        }
    }

    template <class T>
    StaticMap<T> make_static_map()
    {
        StaticMap<T> map;

        for (const StaticLink<T>* p_link : awl::static_chain<T>())
        {
            helpers::insert_static_link(map, p_link);
        }

        return map;
    }

    template <class T>
    StaticMap<T> make_static_map(const std::string& filter)
    {
        if (filter.empty())
        {
            return make_static_map<T>();
        }

        std::basic_regex<char> name_regex(filter);

        StaticMap<T> map;

        for (const StaticLink<T>* p_link : awl::static_chain<T>())
        {
            std::match_results<const char*> match;

            if (std::regex_match(p_link->name(), match, name_regex))
            {
                helpers::insert_static_link(map, p_link);
            }
        }

        return map;
    }
}
