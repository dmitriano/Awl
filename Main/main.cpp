/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Testing/TestConsole.h"
#include "Awl/Testing/TestException.h"
#include "Awl/String.h"
#include "Awl/StdConsole.h"

#include <cstdlib>

#ifdef _MSC_VER
#include <crtdbg.h>
#endif

#ifndef _tmain
#define _tmain main
#endif

namespace
{
    void configureAssertReporting()
    {
#if defined(_MSC_VER) && defined(_DEBUG)
        _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
        _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);

        _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
        _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);

        _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
        _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);

        _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#endif
    }
}

#ifdef AWL_ANSI_CMD_CHAR
int main(int argc, char* argv[])
#else
int _tmain(int argc, awl::Char * argv[])
#endif
{
    configureAssertReporting();

    try
    {
        return awl::testing::run(argc, argv);
    }
    catch (const awl::testing::TestException& e)
    {
        awl::cout() << e.message() << std::endl;
    }

    return 1;
}
