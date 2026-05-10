/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
#pragma once

#include "QtExtras/Json/Jsonable.h"
#include "QtExtras/Json/Json.h"
namespace awl
{
    template <class T>
    class JsonableObject : public Jsonable
    {
    public:

        JsonableObject(T& val) : _val(val) {}

        void fromJson(const QJsonValue& jv) override
        {
            T val;

            //If it throws _val does not change.
            awl::fromJson(jv, val);

            _val = std::move(val);
        }

        virtual QJsonValue toJson() const override
        {
            QJsonValue jv;

            awl::toJson(_val, jv);

            return jv;
        }

    private:

        T& _val;
    };
}
