#pragma once

#include "QtExtras/Json/Jsonable.h"
#include "QtExtras/Json/Json.h"

namespace awl
{
    template <class T>
    class JsonableObject : public Jsonable
    {
    public:

        JsonableObject(T& val) : m_val(val) {}

        void FromJson(const QJsonValue& jv) override
        {
            T val;

            //If it throws m_val does not change.
            awl::FromJson(jv, val);

            m_val = std::move(val);
        }

        virtual QJsonValue ToJson() const override
        {
            QJsonValue jv;

            awl::ToJson(m_val, jv);

            return jv;
        }

    private:

        T& m_val;
    };
}
