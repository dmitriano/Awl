#include "Awl/Testing/TestMap.h"
#include "Awl/Testing/TestAssert.h"

namespace awl
{
    namespace testing
    {
        int RunAllTests(ostream & out)
        {
            int error = 1;

            auto test_map = awl::testing::CreateTestMap();

            try
            {
                out << std::endl << _T("***************** Running all tests *****************") << std::endl;

                test_map->RunAll(out);

                out << std::endl << _T("***************** Tests passed *****************") << std::endl;

                error = 0;
            }
            catch (const std::exception & e)
            {
                out << std::endl << _T("***************** Tests failed: ") << e.what() << std::endl;
            }
            catch (const awl::testing::TestException & e)
            {
                out << std::endl << _T("***************** Tests failed: ") << e.GetMessage() << std::endl;
            }

            if (error != 0)
            {
                out << test_map->GetLastOutput();
            }

            awl::testing::Shutdown();

            return error;
        }
    }
}


