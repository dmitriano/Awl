/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Testing/UnitTest.h"
#include "Awl/StringFormat.h"

using namespace awl::testing;

AWL_EXAMPLE(CppVersion)
{
    awl::format message;

    message << _T("C++ version: ") << __cplusplus << _T(", ");

    if constexpr (__cplusplus >= 202002L) message << "C++20\n";
    else if constexpr (__cplusplus >= 201703L) message << "C++17\n";
    else if constexpr (__cplusplus >= 201402L) message << "C++14\n";
    else if constexpr (__cplusplus >= 201103L) message << "C++11\n";
    else if constexpr (__cplusplus >= 199711L) message << "C++98\n";
    else message << "pre-standard C++\n";

    context.logger.debug(message);
}
