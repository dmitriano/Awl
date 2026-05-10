/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/Testing/TestRunner.h"
#include "Awl/Testing/TestAssert.h"
#include "Awl/Testing/CommandLineProvider.h"
#include "Awl/Testing/LocalAttribute.h"
#include "Awl/IntRange.h"
#include "Awl/StopWatch.h"
#include "Awl/Time.h"
#include "Awl/WatchDog.h"

#include <thread>
#include <functional>
#include <algorithm>
#include <cassert>
#include <memory>

namespace
{
    awl::String toString(const awl::StopWatch& sw)
    {
        awl::ostringstream out;
        out << sw;
        return out.str();
    }

    awl::String toString(std::chrono::milliseconds t)
    {
        awl::ostringstream out;
        awl::format_duration(out, std::chrono::duration_cast<std::chrono::nanoseconds>(t));
        return out.str();
    }
}

namespace awl::testing
{
    TestRunner::TestRunner(std::function<void()> delay_output, std::function<void()> clear_output) :
        _delayOutput(std::move(delay_output)),
        _clearOutput(std::move(clear_output))
    {}

    void TestRunner::runLink(const TestLink* p_test_link, const TestContext& context)
    {
        AWL_ATTRIBUTE(size_t, loop, 0);
        AWL_ATTRIBUTE(std::chrono::milliseconds::rep, timeout, -1);

        // Call std::terminate() when timeout has elapsed.
        // Used for simulating an app crash.
        AWL_FLAG(terminate);

        const String test_name = fromACString(p_test_link->name());
        std::shared_ptr<ILogger> test_logger = context.logger->createLogger(p_test_link->name());

        context.logger->info(_T("{} started."), test_name);

        size_t loop_count = loop;

        if (loop_count != 0)
        {
            context.logger->info(_T("{} looping {} times."), test_name, loop_count);
        }
        else
        {
            loop_count = 1;
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
                    [logger = context.logger, t, terminate]()
                    {
                        if (terminate)
                        {
                            logger->warning(_T("The timeout of {} has elapsed, terminating the app."), toString(t));

                            std::terminate();
                        }
                        else
                        {
                            logger->warning(_T("The timeout of {} has elapsed, requesting the test to stop."), toString(t));
                        }
                    });

                test_token = watch_dog->get_token();
            }
            else
            {
                test_token = context.stopToken;
            }
            
            const TestContext temp_context{ test_logger, test_token, context.attributeProvider, context.typeProvider };

            awl::StopWatch sw;

            if (_delayOutput)
            {
                _delayOutput();
            }

            p_test_link->value()(temp_context);

            if (_clearOutput)
            {
                _clearOutput();
            }

            context.logger->info(_T("{} passed within {}."), test_name, toString(sw));

            // Clear the attributes from the passed test.
            context.attributeProvider.clear();
        }
    }
}
