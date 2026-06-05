#pragma once

#include "BoostExtras/Json/BoostJsonConfig.h"

#include "Awl/Exception.h"
#include "Awl/StringFormat.h"

#include <boost/json.hpp>

#include <format>
#include <string>
#include <vector>

namespace awl::boost_json
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
            GeneralException(std::move(message))
        {
            append(std::move(info));
        }

        void append(ValueInfo info)
        {
            _path.push_back(std::move(info));
        }

        awl::String message() const override
        {
            awl::String text = GeneralException::message();

            if (!_path.empty())
            {
                text += _T("\nDetails:");

                for (auto i = _path.rbegin(); i != _path.rend(); ++i)
                {
                    text += std::format(
                        _T("\n    [{}] ({} / {})"),
                        i->key,
                        kindToString(i->jsonType),
                        i->cppType);
                }
            }

            return text;
        }

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

        std::vector<ValueInfo> _path;
    };
}
