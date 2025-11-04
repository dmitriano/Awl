/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Testing/TestRunner.h"
#include "Awl/Testing/TestAssert.h"
#include "Awl/Testing/CommandLineProvider.h"
#include "Awl/Testing/LocalAttribute.h"

#include "Awl/StdConsole.h"
#include "Awl/IntRange.h"
#include "Awl/StopWatch.h"
#include "Awl/Time.h"
#include "Awl/WatchDog.h"
#include "Awl/ConsoleLogger.h"

#include <thread>
#include <functional>
#include <algorithm>
#include <cassert>

namespace awl::testing
{
    TestRunner::TestRunner(ostringstream& last_output) :
        nullOutput(&nullBuffer),
        lastOutput(last_output)
    {}

    void TestRunner::RunLink(const TestLink* p_test_link, const TestContext& context, awl::ostream& out)
    {
        AWL_ATTRIBUTE(String, output, _T("failed"));
        AWL_ATTRIBUTE(size_t, loop, 0);
        AWL_ATTRIBUTE(std::chrono::milliseconds::rep, timeout, -1);

        // Call std::terminate() when timeout has elapsed.
        // Used for simulating an app crash.
        AWL_FLAG(terminate);

        out << FromACString(p_test_link->name());

        size_t loop_count = loop;

        if (loop_count != 0)
        {
            out << _T(" Looping ") << loop_count << _T(" times.");
        }
        else
        {
            loop_count = 1;
        }

        out << _T("... ");

        //Required on Linux with GCC.
        out.flush();

        std::basic_ostream<Char>* p_out = nullptr;

        if (output == _T("all"))
        {
            out << std::endl;
            
            p_out = &out;
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
                    [&out, t, terminate]()
                    {
                        out << _T("The timeout of ");

                        //TODO: Casting to std::chrono::nanoseconds is a workround. It does not compile with milliseconds.
                        format_duration(out, std::chrono::duration_cast<std::chrono::nanoseconds>(t));

                        out << _T(" has elapsed");

                        if (terminate)
                        {
                            out << _T(", terminating the app...") << std::endl;

                            std::terminate();
                        }
                        else
                        {
                            out << _T(", requesting the test to stop...") << std::endl;
                        }
                    });

                test_token = watch_dog->get_token();
            }
            else
            {
                test_token = context.stopToken;
            }

            ConsoleLogger logger(*p_out);
            
            const TestContext temp_context{ logger, test_token, context.ap };

            awl::StopWatch sw;

            p_test_link->value()(temp_context);

            if (p_out == &lastOutput)
            {
                lastOutput.str(String());
            }

            out << _T("\tPassed within ") << sw << std::endl;

            // Clear the attributes from the passed test.
            context.ap.Clear();
        }
    }
}
