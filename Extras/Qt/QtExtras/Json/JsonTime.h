/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
#pragma once

#include "QtExtras/Json/JsonSerializer.h"
#include "QtExtras/Json/JsonException.h"

#include "Awl/StringFormat.h"

#include <chrono>
#include <format>
#include <sstream>

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

    template <class Rep, class Period>
    class JsonSerializer<std::chrono::duration<Rep, Period>>
    {
    public:

        using value_type = std::chrono::duration<Rep, Period>;
        using formatted_type = std::chrono::hh_mm_ss<value_type>;

        void FromJson(const QJsonValue& jv, value_type& v)
        {
            using common_duration = std::chrono::nanoseconds;

            if (!jv.isString())
            {
                throw JsonException(awl::format() << "Expected duration as JSON string.");
            }

            QString text = jv.toString().trimmed();

            if (text.isEmpty())
            {
                throw JsonException(awl::format() << "Duration string is empty.");
            }

            const QString original_text = text;

            bool negative = false;

            if (text.startsWith('-'))
            {
                negative = true;
                text.remove(0, 1);
            }

            std::istringstream in(text.toStdString());
            common_duration parsed{};

            in >> std::chrono::parse("%T", parsed);

            if (in.fail())
            {
                throw JsonException(awl::format() << "Invalid duration format: " << original_text.toStdString());
            }

            in >> std::ws;

            if (!in.eof())
            {
                throw JsonException(awl::format() << "Unexpected trailing characters in duration: " << original_text.toStdString());
            }

            if (negative)
            {
                parsed = -parsed;
            }

            const value_type converted = std::chrono::duration_cast<value_type>(parsed);

            if (std::chrono::duration_cast<common_duration>(converted) != parsed)
            {
                throw JsonException(awl::format() << "Duration precision loss: " << original_text.toStdString());
            }

            v = converted;
        }

        void ToJson(const value_type& v, QJsonValue& jv)
        {
            const formatted_type formatted(v);
            jv = QString::fromStdString(std::format("{:%T}", formatted));
        }
    };
}
