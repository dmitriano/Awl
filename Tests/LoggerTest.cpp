/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/ConsoleLogger.h"
#include "Awl/StringFormat.h"

#include "Awl/Testing/UnitTest.h"

AWL_TEST(Logger)
{
    awl::ConsoleLogger logger(context.out);

    // Check if it comiles with all the strings.
    logger.debug("abc");
    logger.debug(std::string("abc"));
    logger.debug(awl::aformat() << "abc");

    logger.trace(L"abc");
    logger.trace(std::wstring(L"abc"));
    logger.trace(awl::wformat() << "abc");

    logger.info(awl::String(_T("abc")));
    logger.info(_T("abc"));
    logger.info(awl::format() << "abc");

#ifdef AWL_QT

    logger.warning(QString("abc"));

#endif
}
