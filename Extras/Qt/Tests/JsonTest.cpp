/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
#include "QtExtras/StringConversion.h"
#include "QtExtras/Json/JsonUtil.h"

#include "Tests/VtsData.h"

#include "Awl/Decimal128.h"
#include "Awl/Testing/UnitTest.h"
#include "Awl/StringFormat.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

AWL_TEST(JsonArray)
{
    AWL_UNUSED_CONTEXT;

    QString message = "[1, 2, 3]";
    
    QJsonDocument jdoc = QJsonDocument::fromJson(message.toUtf8());
    QJsonArray a = jdoc.array();

    const int n = a.size();

    AWL_ASSERT(n == 3);
}

namespace
{
    bool IsBinanceError(const awl::testing::TestContext& context, const QString& answer)
    {
        QJsonDocument jdoc = QJsonDocument::fromJson(answer.toUtf8());

        if (jdoc.isObject())
        {
            QJsonObject jobj = jdoc.object();

            //They are not undefined but null.
            //if (!jcode.isUndefined() && !jmsg.isUndefined())
                
            if (jobj.contains("code") && jobj.contains("msg"))
            {
                QJsonValue jcode = jobj["code"];

                QJsonValue jmsg = jobj["msg"];

                context.logger.debug(awl::format() << "Code: " << jcode.toInt() << ", msg: " << awl::FromQString(jmsg.toString()));
                
                return true;
            }
        }

        return false;
    }
}

AWL_TEST(JsonBinanceErrorParsing)
{
    {
        QString err_answer = "{\"code\":-1003,\"msg\":\"Too much request weight used; current limit is 1200 request weight per 1 MINUTE. Please use the websocket for live updates to avoid polling the API.\"}";

        AWL_ASSERT(IsBinanceError(context, err_answer));
    }

    {
        QString other_answer = "{\"id\":-1003}";

        AWL_ASSERT(!IsBinanceError(context, other_answer));
    }
}

using namespace awl::testing::helpers::v1;

namespace
{
    QJsonObject makeAJson()
    {
        QJsonObject jo;

        jo["a"] = a_expected.a;
        jo["b"] = a_expected.b;
        jo["c"] = awl::ToQString(a_expected.c);
        jo["d"] = a_expected.d;

        return jo;
    }

    QJsonObject makeCJson()
    {
        QJsonObject jo;

        jo["x"] = c_expected.x;
        jo["a"] = makeAJson();

        return jo;
    }

    QJsonObject makeBJson()
    {
        QJsonObject jo;

        jo["a"] = makeAJson();
        jo["b"] = makeAJson();
        jo["x"] = b_expected.x;
        jo["y"] = b_expected.y;
        jo["v"] = QJsonArray{ makeAJson(), makeAJson(), makeAJson() };
        jo["v1"] = QJsonArray{ makeCJson() };

        return jo;
    }
}

AWL_TEST(JsonReflectableA)
{
    AWL_UNUSED_CONTEXT;

    A a;

    awl::FromJson(makeAJson(), a);

    AWL_ASSERT(a == a_expected);
}

AWL_TEST(JsonReflectableB)
{
    AWL_UNUSED_CONTEXT;

    B b;

    awl::FromJson(makeBJson(), b);

    AWL_ASSERT(b == b_expected);
}

// Path: a->b
// Message : 'Expected value type: Bool, actul value type: String'
// Details :
//     [a] (Object / struct)
//     [b] (String / int8_t)
AWL_TEST(JsonReflectableExceptionTypeMismatch)
{
    QJsonObject b_jo = makeBJson();

    QJsonObject a_jo = makeAJson();

    a_jo["b"] = "d";

    b_jo["a"] = a_jo;

    B b;

    try
    {
        awl::FromJson(b_jo, b);

        AWL_FAILM(awl::format() << "Exception of type JsonException was not thrown.");
    }
    catch (const awl::JsonException& e)
    {
        context.logger.debug(e.What());
    }
}

// Path: a->b
// Message : 'Expected value type: Bool, actul value type: Null'
// Details :
//     [a] (Object / struct)
//     [b] (Null / int8_t)
AWL_TEST(JsonReflectableExceptionNull)
{
    QJsonObject b_jo = makeBJson();

    QJsonObject a_jo = makeAJson();

    a_jo.erase(a_jo.find("b"));

    b_jo["a"] = a_jo;

    B b;

    try
    {
        awl::FromJson(b_jo, b);

        AWL_FAILM(awl::format() << "Exception of type JsonException was not thrown.");
    }
    catch (const awl::JsonException& e)
    {
        context.logger.debug(e.What());
    }
}

// Path: v->1->b
// Message: 'Expected value type: Bool, actul value type: Null'
// Details:
//  [v] (Array/sequence<struct>)
//  [1] (Object/struct)
//  [b] (Null/int8_t)
AWL_TEST(JsonReflectableExceptionVector)
{
    QJsonObject b_jo = makeBJson();

    QJsonObject a_jo = makeAJson();

    a_jo.erase(a_jo.find("b"));

    b_jo["v"] = QJsonArray{ makeAJson(), a_jo, makeAJson() };

    B b;

    try
    {
        awl::FromJson(b_jo, b);

        AWL_FAILM(awl::format() << "Exception of type JsonException was not thrown.");
    }
    catch (const awl::JsonException& e)
    {
        context.logger.debug(e.What());
    }
}

// Path: v1->1->a->b
// Message: 'Expected value type: Bool, actul value type: String'
// Details:
//  [v1] (Array/sequence<struct>)
//  [1] (Object/struct)
//  [a] (Object/struct)
//  [b] (Null/int8_t)
AWL_TEST(JsonReflectableExceptionSet)
{
    QJsonObject b_jo = makeBJson();

    QJsonObject c_jo = makeCJson();

    QJsonObject a_jo = makeAJson();

    a_jo["b"] = "text";

    c_jo["a"] = a_jo;

    b_jo["v1"] = QJsonArray{ makeCJson(), c_jo, makeCJson() };

    B b;

    try
    {
        awl::FromJson(b_jo, b);

        AWL_FAILM(awl::format() << "Exception of type JsonException was not thrown.");
    }
    catch (const awl::JsonException& e)
    {
        context.logger.debug(e.What());
    }
}

namespace
{
    template <class Map>
    void TestRunner(const awl::testing::TestContext& context)
    {
        static_assert(awl::insertable_map<Map>);

        Map expected_map{ {"a", 0}, {"b", 1}, {"c", 2} };

        QJsonValue jv = awl::ToJson(expected_map);

        {
            Map map;

            awl::FromJson(jv, map);

            AWL_ASSERT(map == expected_map);
        }

        QJsonObject jo = jv.toObject();

        jo["b"] = "text";

        {
            Map map;

            try
            {
                awl::FromJson(jo, map);

                AWL_FAILM(awl::format() << "Exception of type JsonException was not thrown.");
            }
            catch (const awl::JsonException& e)
            {
                context.logger.debug(e.What());
            }
        }
    }
}

AWL_TEST(JsonMap)
{
    TestRunner<std::map<QString, int>>(context);
    TestRunner<std::map<std::string, int>>(context);
    TestRunner<std::unordered_map<QString, int>>(context);
    TestRunner<std::unordered_map<std::string, int>>(context);
}

AWL_TEST(JsonDuration)
{
    using namespace std::chrono;

    auto test_roundtrip = [&context]<class Duration>(const char* label, const auto& values, const auto& to_json_values, const auto& from_json_values)
    {
        for (size_t i = 0; i < values.size(); ++i)
        {
            context.logger.debug(awl::format() << label << " to_json case " << i << ": count=" << values[i].count() << ", json=" << to_json_values[i]);

            {
                QJsonValue jv = awl::ToJson(values[i]);

                AWL_ASSERT(jv.type() == QJsonValue::String);
                AWL_ASSERT(jv.toString() == to_json_values[i]);
            }

            context.logger.debug(awl::format() << label << " from_json case " << i << ": json=" << from_json_values[i] << ", count=" << values[i].count());

            {
                QJsonValue jv = from_json_values[i];

                Duration actual{};

                awl::FromJson(jv, actual);

                AWL_ASSERT(actual == values[i]);
            }
        }
    };

    test_roundtrip.operator()<milliseconds>("milliseconds",
        std::array<milliseconds, 4>
        {
            hours(48) + minutes(2) + seconds(3) + milliseconds(4),
            milliseconds::zero(),
            -(hours(48) + minutes(2) + seconds(3) + milliseconds(4)),
            hours(48) + minutes(2) + seconds(3)
        },
        std::array<QString, 4>
        {
            "48:02:03.004",
            "00:00:00.000",
            "-48:02:03.004",
            "48:02:03.000"
        },
        std::array<QString, 4>
        {
            "48:02:03.004",
            "00:00:00.000",
            "-48:02:03.004",
            "48:02:03"
        });

    test_roundtrip.operator()<microseconds>("microseconds",
        std::array<microseconds, 1>
        {
            hours(48) + minutes(2) + seconds(3) + microseconds(456789)
        },
        std::array<QString, 1>
        {
            "48:02:03.456789"
        },
        std::array<QString, 1>
        {
            "48:02:03.456789"
        });

    test_roundtrip.operator()<seconds>("seconds",
        std::array<seconds, 3>
        {
            hours(48) + minutes(2) + seconds(3),
            seconds::zero(),
            -(hours(48) + minutes(2) + seconds(3))
        },
        std::array<QString, 3>
        {
            "48:02:03",
            "00:00:00",
            "-48:02:03"
        },
        std::array<QString, 3>
        {
            "48:02:03",
            "00:00:00",
            "-48:02:03"
        });

    test_roundtrip.operator()<minutes>("minutes",
        std::array<minutes, 3>
        {
            hours(48) + minutes(2),
            minutes::zero(),
            -(hours(48) + minutes(2))
        },
        std::array<QString, 3>
        {
            "48:02:00",
            "00:00:00",
            "-48:02:00"
        },
        std::array<QString, 3>
        {
            "48:02:00",
            "00:00:00",
            "-48:02:00"
        });

    test_roundtrip.operator()<hours>("hours",
        std::array<hours, 3>
        {
            hours(48),
            hours::zero(),
            -hours(48)
        },
        std::array<QString, 3>
        {
            "48:00:00",
            "00:00:00",
            "-48:00:00"
        },
        std::array<QString, 3>
        {
            "48:00:00",
            "00:00:00",
            "-48:00:00"
        });
}

#ifdef AWL_DECIMAL_128

AWL_TEST(JsonDecimal)
{
    const char* sample = "12345.6789999";
    
    using Decimal = awl::decimal128<4>;

    {
        QJsonValue jv = QString(sample);

        Decimal d;

        awl::FromJson(jv, d);

        AWL_ASSERT(d == Decimal(sample));
    }

    {
        Decimal d(sample);

        QJsonValue jv = awl::ToJson(d);

        AWL_ASSERT(jv.type() == QJsonValue::String);
        AWL_ASSERT(jv.toString() == sample);
    }

    {
        try
        {
            QJsonValue jv = QString("bad.value");

            Decimal d;

            awl::FromJson(jv, d);

            AWL_FAILM(awl::format() << "Exception of type JsonException was not thrown.");
        }
        catch (const awl::JsonException& e)
        {
            context.logger.debug(e.What());
        }
    }
}

#endif
