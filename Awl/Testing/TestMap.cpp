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
#include "Awl/Separator.h"

#include <set>
#include <regex>
#include <thread>

namespace awl::testing
{
    TestMap::TestMap() : nullOutput(&nullBuffer)
    {
        for (TestLink* p_link : GetTestChain())
        {
            if (!testMap.emplace(p_link->GetName(), p_link).second)
            {
                throw TestException(format() << _T("The test '" << p_link->GetName() << _T(" already exists.")));
            }
        }
    }

    void TestMap::Run(const TestContext& context, const Char* name)
    {
        auto i = testMap.find(name);

        if (i == testMap.end())
        {
            throw TestException(format() << _T("The test '" << name << _T(" does not exist.")));
        }

        InternalRun(i->second, context);
    }

    void TestMap::RunAll(const TestContext& context, const std::function<bool(const String&)>& filter)
    {
        for (auto& p : testMap)
        {
            const auto& test_name = p.first;

            if (filter(test_name))
            {
                InternalRun(p.second, context);
            }
        }
    }

    void TestMap::PrintNames(awl::ostream& out, const std::function<bool(const String&)>& filter) const
    {
        for (auto& p : testMap)
        {
            const auto& test_name = p.first;

            if (filter(test_name))
            {
                out << test_name << std::endl;
            }
        }
    }

    void TestMap::InternalRun(TestLink* p_test_link, const TestContext& context)
    {
        AWT_ATTRIBUTE(String, output, _T("failed"));
        AWT_ATTRIBUTE(size_t, loop, 0);
        AWT_ATTRIBUTE(size_t, timeout, 5); //test timeout in seconds

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

        context.out << _T("... ");

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

        std::jthread watch_dog_thread([timeout](std::stop_token token)
        {
            awl::sleep_for(std::chrono::seconds(timeout), token);
        });

        const TestContext temp_context{ *p_out, watch_dog_thread.get_stop_token(), context.ap };

        awl::StopWatch sw;

        for (auto i : awl::make_count(loop_count))
        {
            static_cast<void>(i);

            p_test_link->Run(temp_context);
        }

        context.out << _T("Passed within ");

        auto elapsed = sw.GetElapsedTime();
        
        {
            using namespace std::chrono;

            awl::separator sep(_T(":"));

            const seconds s = duration_cast<seconds>(elapsed);

            if (s != seconds::zero())
            {
                context.out << sep << s;

                elapsed -= s;
            }

            milliseconds ms = duration_cast<milliseconds>(elapsed);

            if (!sep.first() || ms != milliseconds::zero())
            {
                context.out << sep << ms;

                elapsed -= ms;
            }

            context.out << sep << elapsed << std::endl;
        }

        lastOutput.str(String());
    }
}
