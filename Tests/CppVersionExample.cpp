/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Testing/UnitTest.h"

using namespace awl::testing;

AWT_EXAMPLE(CppVersion)
{
    auto& out = context.out;

    out << _T("C++ version: ") << __cplusplus << _T(" ");
    
    if constexpr (__cplusplus == 201703L) out << "C++17\n";
    else if constexpr (__cplusplus == 201402L) out << "C++14\n";
    else if constexpr (__cplusplus == 201103L) out << "C++11\n";
    else if constexpr (__cplusplus == 199711L) out << "C++98\n";
    else out << "pre-standard C++\n";
}
