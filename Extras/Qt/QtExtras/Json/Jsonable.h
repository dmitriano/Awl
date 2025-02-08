#pragma once

#include <QJsonValue>

namespace awl
{
    class Jsonable
    {
    public:

        virtual void FromJson(const QJsonValue& jv) = 0;

        virtual QJsonValue ToJson() const = 0;

        virtual ~Jsonable() = default;
    };
}
