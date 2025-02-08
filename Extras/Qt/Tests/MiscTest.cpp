#include <Awl/Testing/UnitTest.h>

#include <QString>
#include <QDebug>

//Ensure we did not mix release/debug builds.
AWT_TEST(QtMixReleaseDebug)
{
    //AWT_ATTRIBUTE(QString, qstr, QString("QT6"));
    AWT_UNUSED_CONTEXT;

    const QString qstr = "QT6";
    const std::string val = qstr.toStdString();
    //context.out << awl::FromAString(val);
    qDebug() << QString::fromStdString(val);
}
