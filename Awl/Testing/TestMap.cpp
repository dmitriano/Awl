#include "Awl/Testing/TestMap.h"
#include "Awl/Testing/TestAssert.h"
#include "Awl/Testing/CommandLineProvider.h"
#include "Awl/Testing/LocalAttribute.h"

#include "Awl/StdConsole.h"
#include "Awl/IntRange.h"

#include <set>
#include <regex>

namespace awl
{
    namespace testing
    {
        TestMap::TestMap() : nullOutput(&nullBuffer)
        {
            for (TestLink * p_link : GetTestChain())
            {
                if (!testMap.emplace(p_link->GetName(), p_link).second)
                {
                    throw TestException(format() << _T("The test '" << p_link->GetName() << _T(" already exists.")));
                }
            }
        }

        void TestMap::Run(const TestContext & context, const Char * name)
        {
            auto i = testMap.find(name);

            if (i == testMap.end())
            {
                throw TestException(format() << _T("The test '" << name << _T(" does not exist.")));
            }

            InternalRun(i->second, context);
        }

        void TestMap::RunAll(const TestContext & context, const std::function<bool(const String&)> & filter)
        {
            for (auto & p : testMap)
            {
                const auto & test_name = p.first;

                if (filter(test_name))
                {
                    InternalRun(p.second, context);
                }
            }
        }

        void TestMap::PrintNames(awl::ostream & out, const std::function<bool(const String&)> & filter) const
        {
            for (auto & p : testMap)
            {
                const auto & test_name = p.first;

                if (filter(test_name))
                {
                    out << test_name << std::endl;
                }
            }
        }

        void TestMap::InternalRun(TestLink * p_test_link, const TestContext & context)
        {
            AWT_ATTRIBUTE(String, output, _T("failed"));
            AWT_ATTRIBUTE(size_t, loop, 0);
            AWT_FLAG(no_asserts);

            context.out << p_test_link->GetName();

            size_t loop_count = loop;

            if (loop_count != 0)
            {
                context.out << _T(" Looping ") << loop_count << _T(" times.");
            }
            else
            {
                loop_count = 1;
            }

            if (no_asserts)
            {
                context.out << _T(" (Asserts are disabled!) ");
            }

            context.out << _T("...") << std::endl;

            std::basic_ostream<Char> * p_out = nullptr;

            if (output == _T("all"))
            {
                p_out = &context.out;
            }
            else if (output == _T("failed"))
            {
                p_out = &lastOutput;
            }
            else if (output == _T("null"))
            {
                p_out = &nullOutput;
            }
            else
            {
                throw TestException(format() << _T("Not a valid 'output' parameter value: '") << output << _T("'."));
            }

            const TestContext temp_context{ *p_out, context.cancellation, context.ap, !no_asserts };

            for (auto i : awl::make_count(loop_count))
            {
                static_cast<void>(i);
                p_test_link->Run(temp_context);
            }

            lastOutput.str(String());

            context.out << _T("OK") << std::endl;
        }

        std::function<bool(const String &s)> CreateFilter(const String filter)
        {
            if (filter.empty())
            {
                return [](const String &) { return true; };
            }

            return [filter](const String & test_name)
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

        static int RunTests(const TestContext & context)
        {
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

                    for (auto & test : run)
                    {
                        test_map->Run(context, test.c_str());
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
            return RunTests(context);
        }

        int RunAllTests()
        {
            CancellationFlag cancellation;

            AttributeProvider ap;

            const TestContext context{ awl::cout(), cancellation, ap, false };

            return RunAllTests(context);
        }

        int Run(int argc, Char * argv[])
        {
            try
            {
                CommandLineProvider cl(argc, argv);

                CancellationFlag cancellation;

                const TestContext context{ awl::cout(), cancellation, cl, false };

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
            catch (const TestException & e)
            {
                cout() << _T("The following error has occurred: ") << e.GetMessage() << std::endl;
            }

            return 2;
        }
    }
}
