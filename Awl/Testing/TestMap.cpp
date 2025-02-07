/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Testing/TestMap.h"
#include "Awl/Testing/TestAssert.h"
#include "Awl/Testing/CommandLineProvider.h"
#include "Awl/Testing/LocalAttribute.h"

#include "Awl/StdConsole.h"
#include "Awl/IntRange.h"
#include "Awl/StopWatch.h"
#include "Awl/Time.h"
#include "Awl/WatchDog.h"
#include "Awl/ConsoleLogger.h"

#include <set>
#include <regex>
#include <thread>

namespace awl::testing
{
    TestMap::TestMap(ostringstream& last_output, const std::string& filter) :
        nullOutput(&nullBuffer),
        lastOutput(last_output),
        testMap(make_static_map<TestFunc>(filter))
    {}

    void TestMap::Run(const TestContext& context, const char* name)
    {
        auto i = testMap.find(name);

        if (i == testMap.end())
        {
            throw TestException(format() << _T("The test '" << name << _T(" does not exist.")));
        }

        RunLink(i->second, context);
    }

    void TestMap::RunAll(const TestContext& context)
    {
        for (auto& p : testMap)
        {
            RunLink(p.second, context);
        }
    }

    void TestMap::PrintNames(awl::ostream& out) const
    {
        for (auto& p : testMap)
        {
            const auto& test_name = p.first;

            out << test_name << std::endl;
        }
    }

    void TestMap::RunLink(const TestLink* p_test_link, const TestContext& context)
    {
        AWT_ATTRIBUTE(String, output, _T("failed"));
        AWT_ATTRIBUTE(size_t, loop, 0);
        AWT_ATTRIBUTE(std::chrono::milliseconds::rep, timeout, -1);

        context.out << FromACString(p_test_link->name());

        size_t loop_count = loop;

        if (loop_count != 0)
        {
            context.out << _T(" Looping ") << loop_count << _T(" times.");
        }
        else
        {
            loop_count = 1;
        }

        context.out << _T("... ");

        //Required on Linux with GCC.
        context.out.flush();

        std::basic_ostream<Char>* p_out = nullptr;

        if (output == _T("all"))
        {
            context.out << std::endl;
            
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

        for (auto i : awl::make_count(loop_count))
        {
            static_cast<void>(i);

            std::unique_ptr<awl::watch_dog> watch_dog;

            std::stop_token test_token;

            if (timeout >= 0)
            {
                std::chrono::milliseconds t(timeout);
                
                watch_dog = std::make_unique<awl::watch_dog>(context.stopToken, t,
                    [&context, t]()
                    {
                        context.out << _T("The timeout of ");

                        //TODO: Casting to std::chrono::nanoseconds is a workround. It does not compile with milliseconds.
                        format_duration(context.out, std::chrono::duration_cast<std::chrono::nanoseconds>(t));

                        context.out << _T("ms has elapsed, requesting the test to stop...") << std::endl;
                    });

                test_token = watch_dog->get_token();
            }
            else
            {
                test_token = context.stopToken;
            }

            ConsoleLogger logger(*p_out);
            
            const TestContext temp_context{ *p_out, logger, test_token, context.ap };

            awl::StopWatch sw;

            p_test_link->value()(temp_context);

            if (p_out == &lastOutput)
            {
                lastOutput.str(String());
            }

            context.out << _T("\tPassed within ") << sw << std::endl;
        }
    }
}
