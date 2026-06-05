#pragma once

#include "BoostExtras/Json/BoostJsonConfig.h"

#include "BoostExtras/Json/Json.h"
#include "BoostExtras/Json/JsonException.h"
#include "BoostExtras/Json/JsonRange.h"

#include <boost/json.hpp>

#include <string>
#include <string_view>

namespace awl
{
    template <class T>
    void fromJson(const boost::json::value& jv, T& val)
    {
        JsonSerializer<T>().fromJson(jv, val);
    }

    template <class T>
    void toJson(const T& val, boost::json::value& jv)
    {
        JsonSerializer<T>().toJson(val, jv);
    }

    template <class T>
    boost::json::value toJson(const T& val)
    {
        boost::json::value jv;
        JsonSerializer<T>().toJson(val, jv);
        return jv;
    }

    template <class Struct>
    void structFromString(std::string_view text, Struct& val)
    {
        boost::json::value jv = boost::json::parse(text);
        fromJson(jv, val);
    }

    inline std::string jsonToString(const boost::json::value& jv)
    {
        return boost::json::serialize(jv);
    }

    inline std::string jsonToString(const boost::json::object& jo)
    {
        return boost::json::serialize(jo);
    }

    inline std::string jsonToString(const boost::json::array& ja)
    {
        return boost::json::serialize(ja);
    }

    template <class Struct>
    std::string structToString(const Struct& val)
    {
        return jsonToString(toJson(val));
    }
}
