/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
#pragma once

#include "QtExtras/Json/JsonSerializer.h"

#include <chrono>

namespace awl
{
    template <class Clock, class Duration>
    class JsonSerializer<std::chrono::time_point<Clock, Duration>>
    {
    public:

        using value_type = std::chrono::time_point<Clock, Duration>;

        void FromJson(const QJsonValue & jv, value_type & v)
        {
            using namespace std::chrono;

            const double ms = jv.isString() ? std::stod(jv.toString().toStdString()) : jv.toDouble();
            const milliseconds::rep ms_count = static_cast<milliseconds::rep>(ms);
            v = value_type(milliseconds(ms_count));
        }

        void ToJson(const value_type & v, QJsonValue & jv)
        {
            using namespace std::chrono;
            jv = static_cast<double>(duration_cast<milliseconds>(v.time_since_epoch()).count());
        }
    };
}
