#pragma once

#include "BoostExtras/Json/Jsonable.h"
#include "BoostExtras/Json/JsonUtil.h"

#include <boost/json.hpp>

namespace awl
{
    template <class T>
    class JsonableObject : public Jsonable
    {
    public:

        explicit JsonableObject(T& val) :
            _val(val)
        {}

        void fromJson(const boost::json::value& jv) override
        {
            awl::fromJson(jv, _val);
        }

        boost::json::value toJson() const override
        {
            return awl::toJson(_val);
        }

    private:

        T& _val;
    };
}
