#pragma once

#include "Qtil/Jsonable.h"
#include "Qtil/Json/Json.h"

namespace qtil
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
            qtil::FromJson(jv, val);

            m_val = std::move(val);
        }

        virtual QJsonValue ToJson() const override
        {
            QJsonValue jv;

            qtil::ToJson(m_val, jv);

            return jv;
        }

    private:

        T& m_val;
    };
}
