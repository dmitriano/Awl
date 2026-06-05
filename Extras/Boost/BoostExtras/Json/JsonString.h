#pragma once

#include "BoostExtras/Json/JsonHelpers.h"
#include "BoostExtras/Json/JsonSerializer.h"

#include <string>

namespace awl::boost_json
{
    template <>
    class JsonSerializer<std::string>
    {
    public:

        void fromJson(const boost::json::value& jv, std::string& val)
        {
            val = std::string(asString(jv));
        }

        void toJson(const std::string& val, boost::json::value& jv)
        {
            jv = val;
        }
    };

    template <>
    class JsonSerializer<std::wstring>
    {
    public:

        void fromJson(const boost::json::value& jv, std::wstring& val)
        {
            const std::string text(asString(jv));
            val.assign(text.begin(), text.end());
        }

        void toJson(const std::wstring& val, boost::json::value& jv)
        {
            jv = std::string(val.begin(), val.end());
        }
    };
}
