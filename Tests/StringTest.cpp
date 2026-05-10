/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/StringFormat.h"
#include "Awl/Testing/UnitTest.h"

#ifdef AWL_QT

AWL_TEST(StringFormatQt)
{
    const char sample[] = "abc";

    std::ostringstream a_out;
    a_out << sample;
    QString a_str = QString::fromStdString(a_out.str());

    std::wostringstream w_out;
    w_out << sample;
    QString w_str = QString::fromStdWString(w_out.str());

    AWL_ASSERT(a_str == sample);
    AWL_ASSERT(a_str == sample);

    context.logger->debug(_T("{}, {}"), a_str, w_str);
}

#endif //AWL_QT
