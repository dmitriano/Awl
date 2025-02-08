#include "QtExtras/StringConversion.h"
#include "QtExtras/Json/JsonUtil.h"

#include "Tests/VtsData.h"

#include "Awl/Testing/UnitTest.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

AWT_TEST(JsonArray)
{
    AWT_UNUSED_CONTEXT;

    QString message = "[1, 2, 3]";
    
    QJsonDocument jdoc = QJsonDocument::fromJson(message.toUtf8());
    QJsonArray a = jdoc.array();

    const int n = a.size();

    AWT_ASSERT(n == 3);
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

                context.out << "Code: " << jcode.toInt() << ", msg: " << awl::FromQString(jmsg.toString()) << std::endl;
                
                return true;
            }
        }

        return false;
    }
}

AWT_TEST(JsonBinanceErrorParsing)
{
    {
        QString err_answer = "{\"code\":-1003,\"msg\":\"Too much request weight used; current limit is 1200 request weight per 1 MINUTE. Please use the websocket for live updates to avoid polling the API.\"}";

        AWT_ASSERT(IsBinanceError(context, err_answer));
    }

    {
        QString other_answer = "{\"id\":-1003}";

        AWT_ASSERT(!IsBinanceError(context, other_answer));
    }
}

AWT_TEST(JsonReflectable)
{
    AWT_UNUSED_CONTEXT;

    using namespace awl::testing::helpers::v1;

    QJsonObject jo;

    jo["a"] = 1;
    jo["b"] = true;
    jo["c"] = "abc";
    jo["d"] = 2.0;

    A a;

    awl::FromJson(jo, a);

    AWT_ASSERT(a == a_expected);
}
