#include "Awl/StdConsole.h"
#include "Awl/Testing/TestMap.h"
#include "Awl/Testing/TestAssert.h"

int main()
{
    return awl::testing::RunAllTests(awl::cout());
}
