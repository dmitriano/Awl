/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Testing/TestConsole.h"
#include "Awl/Testing/TestMap.h"
#include "Awl/Testing/TestAssert.h"
#include "Awl/Testing/CommandLineProvider.h"
#include "Awl/Testing/LocalAttribute.h"

#include "Awl/StdConsole.h"
#include "Awl/IntRange.h"

#include <set>
#include <regex>

namespace awl::testing
{
    std::function<bool(const String& s)> TestConsole::CreateFilter(const String filter)
    {
        if (filter.empty())
        {
            return [](const String&) { return true; };
        }

        return [filter](const String& test_name)
        {
            try
            {
                std::basic_regex<Char> test_name_regex(filter);

                std::match_results<String::const_iterator> match;

                return std::regex_match(test_name, match, test_name_regex);
            }
            catch (const std::regex_error&)
            {
                throw TestException(format() << _T("Not a valid regular expression '") << filter << _T("'."));
            }
        };
    }

    int TestConsole::RunTests(const TestContext& context)
    {
        AWT_ATTRIBUTE(size_t, timeout, default_cancellation_timeout); //test timeout in seconds

        m_cancellation.SetTimeout(std::chrono::seconds(timeout));
        
        int error = 1;

        auto test_map = awl::testing::CreateTestMap();

        AWT_ATTRIBUTE(std::set<String>, run, {});

        try
        {
            if (run.empty())
            {
                AWT_ATTRIBUTE(String, filter, _T(".*_Test"));

                auto f = CreateFilter(filter);

                context.out << std::endl << _T("***************** Running ") << test_map->GetCount(f) << _T(" tests *****************") << std::endl;

                test_map->RunAll(context, f);
            }
            else
            {
                context.out << std::endl << _T("***************** Running ") << run.size() << _T(" tests *****************") << std::endl;

                for (auto& test : run)
                {
                    test_map->Run(context, test.c_str());
                }
            }

            context.out << std::endl << _T("***************** The tests passed *****************") << std::endl;

            error = 0;
        }
        catch (const awl::testing::TestException& e)
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

    int TestConsole::RunAllTests()
    {
        AttributeProvider ap;

        const TestContext context{ awl::cout(), m_cancellation, ap };

        return RunTests(context);
    }

    int TestConsole::Run(int argc, Char* argv[])
    {
        try
        {
            CommandLineProvider cl(argc, argv);

            const TestContext context{ awl::cout(), m_cancellation, cl };

            AWT_FLAG(list);

            if (list)
            {
                auto test_map = awl::testing::CreateTestMap();

                AWT_ATTRIBUTE(String, filter, {});

                auto f = CreateFilter(filter);

                test_map->PrintNames(awl::cout(), f);

                awl::cout() << _T("Total ") << test_map->GetCount(f) << _T(" tests.") << std::endl;

                return 0;
            }

            return RunTests(context);
        }
        catch (const TestException& e)
        {
            cout() << _T("The following error has occurred: ") << e.GetMessage() << std::endl;
        }

        return 2;
    }
}
