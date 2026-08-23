#pragma once

#include "BoostExtras/Json/BoostJsonConfig.h"

#include <boost/json.hpp>

namespace awl
{
    class Jsonable
    {
    public:

        virtual ~Jsonable() = default;

        virtual void fromJson(const boost::json::value& jv) = 0;

        virtual boost::json::value toJson() const = 0;
    };
}
