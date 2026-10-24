#pragma once

#include "BoostExtras/Json/JsonHelpers.h"
#include "BoostExtras/Json/JsonSerializer.h"

#include <boost/locale/encoding_utf.hpp>
#include <string>

namespace awl
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
            const std::string_view text = asString(jv);
            try
            {
                val = boost::locale::conv::utf_to_utf<wchar_t>(text.data(), text.data() + text.size(), boost::locale::conv::stop);
            }
            catch (const boost::locale::conv::conversion_error&)
            {
                throw JsonException("Invalid UTF-8 string.");
            }
        }

        void toJson(const std::wstring& val, boost::json::value& jv)
        {
            try
            {
                jv = boost::locale::conv::utf_to_utf<char>(val.data(), val.data() + val.size(), boost::locale::conv::stop);
            }
            catch (const boost::locale::conv::conversion_error&)
            {
                throw JsonException("Invalid wide Unicode string.");
            }
        }
    };
}
