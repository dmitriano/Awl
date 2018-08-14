#include "Awl/StdConsole.h"
#include "Awl/Testing/TestMap.h"
#include "Awl/Testing/TestAssert.h"
#include "Awl/Testing/CommandLineProvider.h"

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

        int Run(int argc, Char * argv[])
        {
            try
            {
                CommandLineProvider cl(argc, argv);

                Cancellation cancellation;

                const TestContext context{ awl::cout(), cancellation, cl };

                return awl::testing::RunAllTests(context);
            }
            catch (const awl::testing::TestException & e)
            {
                cout() << _T("The following error has occurred while parsing the command line arguments: ") << e.GetMessage() << std::endl;
            }

            return 2;
        }
    }
}
