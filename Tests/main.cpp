/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef AWL_COMPILE_MAIN

#include "Awl/Testing/TestConsole.h"
#include "Awl/Testing/TestException.h"
#include "Awl/String.h"
#include "Awl/StdConsole.h"

#ifndef _tmain
#define _tmain main
#endif

int _tmain(int argc, awl::Char * argv[])
{
    try
    {
        return awl::testing::Run(argc, argv);
    }
    catch (const awl::testing::TestException& e)
    {
        awl::cout() << e.GetMessage() << std::endl;
    }
}

#endif
