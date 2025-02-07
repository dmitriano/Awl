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
#include "Awl/ScopeGuard.h"

#include <set>
#include <regex>

namespace awl::testing
{
    int Run(int argc, Char* argv[])
    {
        CompositeProvider<CommandLineProvider> ap(CommandLineProvider(argc, argv));

        TestConsole console(ap);

        // TODO: call PrintUnusedOptions()
        // auto guard = make_scope_guard([&ap] { ap.PrintUnusedOptions()})

        return console.Run();
    }

    int Run()
    {
        return Run(0, nullptr);
    }

    TestConsole::TestConsole(CompositeProvider<CommandLineProvider>& ap) :
        m_ap(ap),
        m_context{ awl::cout(), m_logger, m_source.get_token(), m_ap }
    {
    }

    std::function<bool(const std::string& s)> TestConsole::CreateFilter(const std::string& filter)
    {
        if (filter.empty())
        {
            return [](const std::string&) { return true; };
        }

        return [filter](const std::string& test_name)
        {
            try
            {
                std::basic_regex<char> test_name_regex(filter);

                std::match_results<std::string::const_iterator> match;

                return std::regex_match(test_name, match, test_name_regex);
            }
            catch (const std::regex_error&)
            {
                throw TestException(format() << _T("Not a valid regular expression '") << filter << _T("'."));
            }
        };
    }

    bool TestConsole::RunTests()
    {
        TestContext& context = m_context;
        
        AWT_FLAG(list);

        ostringstream last_output;

        if (list)
        {
            AWT_ATTRIBUTE(std::string, filter, {});

            TestMap test_map(last_output, filter);

            test_map.PrintNames(awl::cout());

            awl::cout() << _T("Total ") << test_map.GetCount() << _T(" tests.") << std::endl;

            return 0;
        }

        AWT_ATTRIBUTE(std::set<String>, run, {});

        bool passed = false;

        try
        {
            if (run.empty())
            {
                AWT_ATTRIBUTE(std::string, filter, ".*_Test");

                TestMap test_map(last_output, filter);

                context.out << std::endl << _T("***************** Running ") << test_map.GetCount() << _T(" tests *****************") << std::endl;

                test_map.RunAll(context);
            }
            else
            {
                TestMap test_map(last_output, "");

                context.out << std::endl << _T("***************** Running ") << run.size() << _T(" tests *****************") << std::endl;

                for (auto& test : run)
                {
                    test_map.Run(context, ToAString(test).c_str());
                }
            }

            context.out << std::endl << _T("***************** The tests passed *****************") << std::endl;

            passed = true;
        }
        catch (const awl::testing::TestException& e)
        {
            context.out << std::endl << last_output.str();

            context.out << std::endl << _T("***************** The tests failed: ") << e.What() << std::endl;
        }

        // awl::static_chain<TestFunc>().clear();

        return passed;
    }

    int TestConsole::Run()
    {
        try
        {
            return RunTests();
        }
        catch (const TestException& e)
        {
            cout() << _T("The following error has occurred: ") << e.What() << std::endl;
        }

        return 2;
    }
}
