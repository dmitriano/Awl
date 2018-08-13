#include "Awl/Testing/TestMap.h"
#include "Awl/Testing/TestAssert.h"
#include "Awl/StdConsole.h"

namespace awl
{
    namespace testing
    {
        int RunAllTests(const TestContext & context)
        {
            int error = 1;

            auto test_map = awl::testing::CreateTestMap();

            try
            {
                context.out << std::endl << _T("***************** Running all tests *****************") << std::endl;

                test_map->RunAll(context);

                context.out << std::endl << _T("***************** Tests passed *****************") << std::endl;

                error = 0;
            }
            catch (const std::exception & e)
            {
                context.out << std::endl << _T("***************** Tests failed: ") << e.what() << std::endl;
            }
            catch (const awl::testing::TestException & e)
            {
                context.out << std::endl << _T("***************** Tests failed: ") << e.GetMessage() << std::endl;
            }

            if (error != 0)
            {
                context.out << test_map->GetLastOutput();
            }

            awl::testing::Shutdown();

            return error;
        }

        int RunAllTests()
        {
            Cancellation cancellation;

            AttributeProvider ap;

            const TestContext context{ awl::cout(), cancellation, ap };

            return RunAllTests(context);
        }
    }
}
