/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
#include <Awl/Testing/UnitTest.h>

#include <QString>
#include <QDebug>

//Ensure we did not mix release/debug builds.
AWL_TEST(QtMixReleaseDebug)
{
    //AWL_ATTRIBUTE(QString, qstr, QString("QT6"));
    AWL_UNUSED_CONTEXT;

    const QString qstr = "QT6";
    const std::string val = qstr.toStdString();
    //context.out << awl::FromAString(val);
    qDebug() << QString::fromStdString(val);
}
