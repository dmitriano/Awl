#include "Awl/StdConsole.h"
#include "Awl/Testing/TestMap.h"
#include "Awl/Testing/TestAssert.h"
#include "Awl/Testing/CommandLineProvider.h"
#include "Awl/Testing/LocalAttribute.h"

#include <set>

namespace awl
{
    namespace testing
    {
        void TestMap::Run(const TestContext & context, const String & name)
        {
            auto i = testMap.find(name);

            if (i == testMap.end())
            {
                ostringstream out;

                out << _T("The test '" << name << _T(" does not exist."));

                throw TestException(out.str());
            }

            InternalRun(i->second, context);
        }
            
        void TestMap::RunAll(const TestContext & context)
        {
            for (auto & p : testMap)
            {
                InternalRun(p.second, context);
            }
        }

        void TestMap::InternalRun(TestLink * p_test_link, const TestContext & context)
        {
            context.out << p_test_link->GetName() << _T("...");

            const TestContext temp_context{ lastOutput, context.cancellation, context.ap };

            p_test_link->Run(temp_context);

            context.out << _T("OK") << std::endl;

            lastOutput.clear();
        }

        static int RunTests(const TestContext & context, const std::set<String> & tests)
        {
            int error = 1;

            auto test_map = awl::testing::CreateTestMap();

            try
            {
                const size_t test_count = tests.empty() ? test_map->GetTestCount() : tests.size();
                
                context.out << std::endl << _T("***************** Running ") << test_count << _T(" tests *****************") << std::endl;

                if (tests.empty())
                {
                    test_map->RunAll(context);
                }
                else
                {
                    for (auto & test : tests)
                    {
                        test_map->Run(context, test);
                    }
                }

                context.out << std::endl << _T("***************** The tests passed *****************") << std::endl;

                error = 0;
            }
            catch (const awl::testing::TestException & e)
            {
                context.out << std::endl << _T("***************** The tests failed: ") << e.GetMessage() << std::endl;
            }

            if (error != 0)
            {
                context.out << test_map->GetLastOutput();
            }

            awl::testing::Shutdown();

            return error;
        }

        int RunAllTests(const TestContext & context)
        {
            return RunTests(context, {});
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

                AWL_ATTRIBUTE(std::set<String>, run, {});
                
                return RunTests(context, run);
            }
            catch (const TestException & e)
            {
                cout() << _T("The following error has occurred: ") << e.GetMessage() << std::endl;
            }

            return 2;
        }
    }
}
