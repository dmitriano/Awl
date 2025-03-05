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

    QString a_str = awl::aformat() << sample;

    QString w_str = awl::wformat() << sample;

    AWL_ASSERT(a_str == sample);
    AWL_ASSERT(a_str == sample);

    context.logger.debug(awl::format() << a_str << ", " << w_str);
}

#endif //AWL_QT
