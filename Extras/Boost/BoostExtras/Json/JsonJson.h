#pragma once

#include "BoostExtras/Json/JsonHelpers.h"
#include "BoostExtras/Json/JsonSerializer.h"

namespace awl::boost_json
{
    template <>
    class JsonSerializer<boost::json::value>
    {
    public:

        void fromJson(const boost::json::value& jv, boost::json::value& val)
        {
            val = jv;
        }

        void toJson(const boost::json::value& val, boost::json::value& jv)
        {
            jv = val;
        }
    };

    template <>
    class JsonSerializer<boost::json::object>
    {
    public:

        void fromJson(const boost::json::value& jv, boost::json::object& val)
        {
            val = asObject(jv);
        }

        void toJson(const boost::json::object& val, boost::json::value& jv)
        {
            jv = val;
        }
    };

    template <>
    class JsonSerializer<boost::json::array>
    {
    public:

        void fromJson(const boost::json::value& jv, boost::json::array& val)
        {
            val = asArray(jv);
        }

        void toJson(const boost::json::array& val, boost::json::value& jv)
        {
            jv = val;
        }
    };
}
