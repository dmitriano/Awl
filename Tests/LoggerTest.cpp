/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/ConsoleLogger.h"
#include "Awl/StringFormat.h"

#include "Awl/Testing/UnitTest.h"

AWL_TEST(Logger)
{
    // Check if it comiles with all the strings.
    context.logger.debug("abc");
    context.logger.debug(std::string("abc"));
    context.logger.debug(awl::aformat() << "abc");

    context.logger.trace(L"abc");
    context.logger.trace(std::wstring(L"abc"));
    context.logger.trace(awl::wformat() << "abc");

    context.logger.info(awl::String(_T("abc")));
    context.logger.info(_T("abc"));
    context.logger.info(awl::format() << "abc");

#ifdef AWL_QT

    context.logger.warning(QString("abc"));

#endif
}
