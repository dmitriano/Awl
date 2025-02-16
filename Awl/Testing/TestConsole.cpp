/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "QtExtras/Json/JsonLoadSave.h"
#include "QtExtras/StringConversion.h"

#include "Awl/Testing/TestConsole.h"
#include "Awl/Testing/TestMap.h"
#include "Awl/Testing/TestAssert.h"
#include "Awl/Testing/CommandLineProvider.h"
#include "Awl/Testing/LocalAttribute.h"

#include "Awl/StdConsole.h"
#include "Awl/IntRange.h"
#include "Awl/ScopeGuard.h"
#include "Awl/StdConsole.h"

#include <set>
#include <regex>

namespace awl::testing
{
    template <attribute_provider Provider>
    TestConsole<Provider>::TestConsole(Provider& ap) :
        m_ap(ap),
        m_context{ awl::cout(), m_logger, m_source.get_token(), m_ap }
    {
    }

    template <attribute_provider Provider>
    bool TestConsole<Provider>::RunTests()
    {
        TestContext& context = m_context;
        
        AWT_FLAG(list);

        if (list)
        {
            AWT_ATTRIBUTE(std::string, filter, {});

            auto test_map = make_static_map<TestFunc>(filter);

            for (auto& p : test_map)
            {
                const auto& test_name = p.first;

                awl::cout() << test_name << std::endl;
            }

            awl::cout() << _T("Total ") << test_map.size() << _T(" tests.") << std::endl;

            return 0;
        }

        AWT_ATTRIBUTE(std::set<String>, run, {});

        bool passed = false;

        ostringstream last_output;

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

    template <attribute_provider Provider>
    int TestConsole<Provider>::Run()
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

    int Run(int argc, Char* argv[])
    {
        CommandLineProvider cl(argc, argv);

#ifdef AWL_QT

        QJsonObject jo;

        String json_file;

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

        CompositeProvider<CommandLineProvider, JsonProvider> ap(std::move(cl), JsonProvider(std::move(jo)));

#else

        CompositeProvider<CommandLineProvider> ap(std::move(cl));

#endif

        TestConsole console(ap);

        // TODO: call PrintUnusedOptions()
        // auto guard = make_scope_guard([&ap] { ap.PrintUnusedOptions()})

        return console.Run();
    }

    int Run()
    {
        return Run(0, nullptr);
    }
}
