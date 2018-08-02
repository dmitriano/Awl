#include <iostream>

#include "UnitTesting.h"

int main()
{
    int error = 1;
    
    auto test_map = awl::testing::CreateTestMap();

    try
    {
        std::cout << std::endl << "***************** Running all tests *****************" << std::endl;

        test_map->RunAll();

        std::cout << std::endl << "***************** Tests passed *****************" << std::endl;

        error = 0;
    }
    catch (const std::exception & e)
    {
        std::cout << std::endl << "***************** Tests failed: " << e.what() << std::endl;

        std::cout << test_map->GetLastOutput();
    }

    awl::testing::Shutdown();

    return error;
}
