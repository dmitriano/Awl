/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Testing/TestConsole.h"
#include "Awl/Testing/TestRunner.h"
#include "Awl/Testing/TestAssert.h"
#include "Awl/Testing/CommandLineProvider.h"
#include "Awl/Testing/LocalAttribute.h"

#include "Awl/StdConsole.h"
#include "Awl/IntRange.h"
#include "Awl/ScopeGuard.h"
#include "Awl/StaticMap.h"

#ifdef AWL_QT
    #include "QtExtras/Json/JsonLoadSave.h"
    #include "QtExtras/StringConversion.h"
#endif

#include <set>
#include <regex>

namespace awl::testing
{
    template <attribute_provider Provider>
    TestConsole<Provider>::TestConsole(Provider& ap, std::stop_token token) :
        m_ap(ap),
        m_context{ m_logger, std::move(token), m_ap}
    {
    }

    template <attribute_provider Provider>
    bool TestConsole<Provider>::RunTests()
    {
        TestContext& context = m_context;

        awl::ostream& out = awl::cout();
        
        AWL_ATTRIBUTE(std::string, run, {});

        bool passed = false;

        ostringstream last_output;

        try
        {
            TestRunner runner(last_output);

            if (run.empty())
            {
                AWL_ATTRIBUTE(std::string, filter, ".*_Test");

                StaticMap<TestFunc> test_map{ StaticMap<TestFunc>::fill(filter) };

                out << std::endl << _T("***************** Running ") << test_map.size() << _T(" tests *****************") << std::endl;

                for (const TestLink* p_link : test_map)
                {
                    runner.RunLink(p_link, context, out);
                }
            }
            else
            {
                out << std::endl << _T("***************** Running test ") << run << _T(" *****************") << std::endl;

                const TestLink* p_link = static_chain<TestFunc>().find(run.c_str());

                if (p_link == nullptr)
                {
                    throw TestException(format() << _T("The test '" << run << _T(" does not exist.")));
                }

                runner.RunLink(p_link, context, out);
            }

            out << std::endl << _T("***************** The tests passed *****************") << std::endl;

            passed = true;
        }
        catch (const awl::testing::TestException& e)
        {
            out << std::endl << last_output.str();

            out << std::endl << _T("***************** The tests failed: ") << e.What() << std::endl;
        }

        // awl::static_chain<TestFunc>().clear();

        return passed;
    }

    template <attribute_provider Provider>
    int TestConsole<Provider>::Run()
    {
        try
        {
            const bool passed = RunTests();

            return passed ? 0 : 1;
        }
        catch (const TestException& e)
        {
            cout() << _T("The following error has occurred: ") << e.What() << std::endl;
        }

        return 2;
    }

    int Run(int argc, CmdChar* argv[], std::stop_token token)
    {
        CommandLineProvider cl(argc, argv);

        // "list" command runs without TestRunner
        {
            ProviderContext<CommandLineProvider> context{ cl };

            AWL_FLAG(list);

            if (list)
            {
                AWL_ATTRIBUTE(std::string, filter, {});

                auto test_map = StaticMap<TestFunc>::fill(filter);

                for (auto& p_link : test_map)
                {
                    awl::cout() << p_link->name() << std::endl;
                }

                awl::cout() << _T("Total ") << test_map.size() << _T(" tests.") << std::endl;

                return 0;
            }
        }

#ifdef AWL_QT

        QJsonObject jo;

        CmdString json_file;

        if (cl.TryGet("json", json_file))
        {
            try
            {
                jo = loadObjectFromFile(ToQString(json_file));
            }
            catch (const JsonException& e)
            {
                awl::cout() << e.What() << std::endl;

                return 3;
            }
        }

        CompositeProvider<CommandLineProvider, JsonProvider> ap(std::move(cl), JsonProvider(jo));

#else

        CompositeProvider<CommandLineProvider> ap(std::move(cl));

#endif

        TestConsole console(ap, std::move(token));

        auto guard = make_scope_guard([&ap]
        {
            auto names = ap.get_provider<0>().GetUnusedOptions();

            for (const std::string& name : names)
            {
                cout() << _T("Unused option '" << name << _T("'")) << std::endl;
            }
        });

        return console.Run();
    }

    int Run(int argc, CmdChar* argv[])
    {
        std::stop_source source;

        return Run(argc, argv, source.get_token());
    }

    int Run()
    {
        std::stop_source source;

        return Run(source.get_token());
    }

    int Run(std::stop_token token)
    {
        return Run(0, nullptr, std::move(token));
    }
}
