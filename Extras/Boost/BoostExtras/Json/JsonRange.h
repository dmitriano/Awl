#pragma once

#include "BoostExtras/Json/BoostJsonConfig.h"

#include "BoostExtras/Json/JsonSerializer.h"

#include <boost/json.hpp>

#include <ranges>

namespace awl::boost_json
{
    template <std::ranges::range Range>
    boost::json::array rangeToJson(const Range& r)
    {
        using T = std::ranges::range_value_t<Range>;
        JsonSerializer<T> formatter;
        boost::json::array ja;

        for (const T& val : r)
        {
            boost::json::value jv;
            formatter.toJson(val, jv);
            ja.push_back(std::move(jv));
        }

        return ja;
    }
}
