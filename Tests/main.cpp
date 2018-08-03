#include "Awl/StdConsole.h"
#include "Awl/Testing/TestMap.h"

int main()
{
    int error = 1;
    
    auto test_map = awl::testing::CreateTestMap();

    try
    {
        awl::cout() << std::endl << _T("***************** Running all tests *****************") << std::endl;

        test_map->RunAll();

        awl::cout() << std::endl << _T("***************** Tests passed *****************") << std::endl;

        error = 0;
    }
    catch (const std::exception & e)
    {
        awl::cout() << std::endl << _T("***************** Tests failed: ") << e.what() << std::endl;

        awl::cout() << test_map->GetLastOutput();
    }

    awl::testing::Shutdown();

    return error;
}
