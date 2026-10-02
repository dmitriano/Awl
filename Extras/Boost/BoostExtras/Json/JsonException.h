#pragma once

#include "BoostExtras/Json/BoostJsonConfig.h"

#include "Awl/Exception.h"
#include "Awl/StringFormat.h"

#include <boost/json.hpp>

#include <format>
#include <string>
#include <string_view>

namespace awl
{
    class JsonException : public awl::GeneralException
    {
    public:

        struct ValueInfo
        {
            boost::json::kind jsonType;
            std::string cppType;
            std::string key;
        };

        using GeneralException::GeneralException;

        JsonException(awl::String message, ValueInfo info) :
            JsonException(JsonException(std::move(message)), std::move(info))
        {}

        JsonException(const JsonException& cause, ValueInfo info) :
            GeneralException(addContext(cause, info)),
            _detailsPos(cause._detailsPos == std::string::npos ? cause._message.size() : cause._detailsPos)
        {}

        static awl::String kindToString(boost::json::kind kind)
        {
            switch (kind)
            {
            case boost::json::kind::null: return _T("Null");
            case boost::json::kind::bool_: return _T("Bool");
            case boost::json::kind::int64: return _T("Int64");
            case boost::json::kind::uint64: return _T("UInt64");
            case boost::json::kind::double_: return _T("Double");
            case boost::json::kind::string: return _T("String");
            case boost::json::kind::array: return _T("Array");
            case boost::json::kind::object: return _T("Object");
            }

            throw JsonException(std::format(_T("Wrong JSON kind value: {}."), static_cast<int>(kind)));
        }

    private:

        static std::string addContext(const JsonException& cause, const ValueInfo& info)
        {
            std::string text = cause._message;
            constexpr std::string_view details = "\nDetails:";
            std::size_t pos = cause._detailsPos;

            if (pos == std::string::npos)
            {
                pos = text.size();
                text += details;
            }

            // Each new outer context precedes the existing inner contexts.
            text.insert(pos + details.size(), std::format("\n    [{}] ({} / {})",
                info.key, toAString(kindToString(info.jsonType)), info.cppType));

            return text;
        }

        // Byte offset of the details section, so the full text is stored only once.
        std::size_t _detailsPos = std::string::npos;
    };
}
