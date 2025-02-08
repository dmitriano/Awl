#pragma once

#include "Awl/Exception.h"
#include "Awl/StringFormat.h"

#include <QJsonValue>

#include <vector>

namespace awl
{
    class JsonException : public GeneralException
    {
    public:

        struct ValueInfo
        {
            QJsonValue::Type jsonType;
            std::string cppType;
            std::string key;
        };

        using GeneralException::GeneralException;

        JsonException(String message, ValueInfo info);

        void append(ValueInfo info);

        String What() const override;

    private:

        std::vector<ValueInfo> m_path;
    };
}
