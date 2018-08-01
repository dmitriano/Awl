#include <iostream>

#include "UnitTesting.h"

int main()
{
    int error = 1;
    
    try
    {
        std::cout << std::endl << "***************** Running all tests *****************" << std::endl;

        auto test_map = awl::testing::CreateTestMap();

        test_map->RunAll();

        std::cout << std::endl << "***************** Tests passed *****************" << std::endl;

        error = 0;
    }
    catch (const std::exception & e)
    {
        std::cout << std::endl << "***************** Tests failed: " << e.what() << std::endl;
    }

    awl::testing::Shutdown();

    return error;
}
