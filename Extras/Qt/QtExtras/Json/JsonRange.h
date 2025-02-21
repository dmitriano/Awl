/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
#pragma once

#include "QtExtras/Json/JsonSerializer.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

#include <ranges>

namespace awl
{
    template <std::ranges::range Range>
    QJsonArray RangeToJson(const Range& r)
    {
        using T = std::ranges::range_value_t<Range>;

        JsonSerializer<T> formatter;

        QJsonArray ja;

        for (auto& val : r)
        {
            QJsonValue j_val;

            formatter.ToJson(val, j_val);

            ja.append(j_val);
        }

        return ja;
    }
}
