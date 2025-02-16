/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Testing/TestConsole.h"
#include "Awl/Testing/TestException.h"
#include "Awl/String.h"
#include "Awl/StdConsole.h"

#ifndef _tmain
#define _tmain main
#endif

#ifdef AWL_ANSI_CMD_CHAR
int main(int argc, char* argv[])
#else
int _tmain(int argc, awl::Char * argv[])
#endif
{
    try
    {
        return awl::testing::Run(argc, argv);
    }
    catch (const awl::testing::TestException& e)
    {
        awl::cout() << e.What() << std::endl;
    }
}
