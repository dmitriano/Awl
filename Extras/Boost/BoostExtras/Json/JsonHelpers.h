#pragma once

#include "BoostExtras/Json/BoostJsonConfig.h"

#include "BoostExtras/Json/JsonException.h"

#include <boost/json.hpp>

#include <format>
#include <string>

namespace awl
{
    inline awl::String typeToString(boost::json::kind kind)
    {
        return JsonException::kindToString(kind);
    }

    inline void ensureType(const boost::json::value& jv, boost::json::kind kind)
    {
        if (jv.kind() != kind)
        {
            throw JsonException(std::format(
                _T("Expected value type: {}, actual value type: {}"),
                typeToString(kind),
                typeToString(jv.kind())));
        }
    }

    inline bool isNull(const boost::json::value& jv)
    {
        return jv.is_null();
    }

    inline const boost::json::object& asObject(const boost::json::value& jv)
    {
        ensureType(jv, boost::json::kind::object);
        return jv.as_object();
    }

    inline const boost::json::array& asArray(const boost::json::value& jv)
    {
        ensureType(jv, boost::json::kind::array);
        return jv.as_array();
    }

    inline boost::json::string_view asString(const boost::json::value& jv)
    {
        ensureType(jv, boost::json::kind::string);
        return jv.as_string();
    }
}
