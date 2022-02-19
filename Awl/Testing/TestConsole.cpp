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
    int Run()
    {
        AttributeProvider ap;

        TestConsole console(ap);

        return console.Run();
    }

    int Run(int argc, Char* argv[])
    {
        CommandLineProvider ap(argc, argv);

        TestConsole console(ap);

        return console.Run();
    }

    TestConsole::TestConsole(AttributeProvider& ap) :
        m_ap(ap),
        m_context{ awl::cout(), m_source.get_token(), m_ap }
    {
    }

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

    bool TestConsole::RunTests()
    {
        TestContext& context = m_context;
        
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

        auto test_map = awl::testing::CreateTestMap();

        AWT_ATTRIBUTE(std::set<String>, run, {});

        bool passed = false;
        
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

            passed = true;
        }
        catch (const awl::testing::TestException& e)
        {
            context.out << std::endl << test_map->GetLastOutput();

            context.out << std::endl << _T("***************** The tests failed: ") << e.GetMessage() << std::endl;
        }

        awl::testing::Shutdown();

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
            cout() << _T("The following error has occurred: ") << e.GetMessage() << std::endl;
        }

        return 2;
    }
}
