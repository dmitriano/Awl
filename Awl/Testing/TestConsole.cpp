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
#include "Awl/CompositeLogger.h"
#include "Awl/IntRange.h"
#include "Awl/ScopeGuard.h"
#include "Awl/StaticMap.h"
#include "Awl/StdStreamLogger.h"

#ifdef AWL_QT
    #include "QtExtras/Json/JsonLoadSave.h"
    #include "QtExtras/StringConversion.h"
#endif

#include <filesystem>
#include <fstream>
#include <optional>
#include <set>
#include <regex>

namespace awl::testing
{
    namespace
    {
        std::shared_ptr<CompositeLogger> makeStdoutLogger(const std::string& log_level)
        {
            auto logger = std::make_shared<CompositeLogger>();
            logger->addLogger(std::make_shared<StdStreamLogger>(
                "TestConsole",
                StdStreamLogger::coutStream(),
                log_level));
            return logger;
        }

        void addFileLogger(
            const std::shared_ptr<CompositeLogger>& logger,
            const std::optional<std::string>& log_file,
            const std::string& log_level)
        {
            if (!log_file)
            {
                return;
            }

            auto file_out = std::make_shared<std::basic_ofstream<Char>>(
                std::filesystem::path(*log_file),
                std::ios_base::app);

            if (!*file_out)
            {
                throw TestException(std::format("Cannot open log file '{}'.", *log_file));
            }

            logger->addLogger(std::make_shared<StdStreamLogger>(
                "TestConsole",
                file_out,
                log_level));
        }

        std::shared_ptr<CompositeLogger> makeTestConsoleLogger(
            const String& output,
            const std::string& log_level,
            const std::optional<std::string>& log_file,
            ostringstream& last_output)
        {
            auto logger = std::make_shared<CompositeLogger>();

            if (output == _T("all"))
            {
                logger->addLogger(std::make_shared<StdStreamLogger>(
                    "TestConsole",
                    StdStreamLogger::coutStream(),
                    log_level));
            }
            else if (output == _T("failed"))
            {
                logger->addLogger(std::make_shared<StdStreamLogger>(
                    "TestConsole",
                    StdStreamLogger::wrapStream(last_output),
                    log_level));
            }
            else if (output != _T("null"))
            {
                throw TestException(std::format(_T("Not a valid 'output' parameter value: '{}'."), output));
            }

            addFileLogger(logger, log_file, log_level);

            return logger;
        }
    }

    template <attribute_provider Provider>
    TestConsole<Provider>::TestConsole(Provider& provider, std::stop_token stop_token) :
        _logger(makeStdoutLogger(LogLevel::Trace)),
        _ap(provider),
        _context{ _logger, std::move(stop_token), _ap, _typeProvider}
    {}

    template <attribute_provider Provider>
    bool TestConsole<Provider>::runTests()
    {
        TestContext& context = _context;
        
        AWL_ATTRIBUTE(String, output, _T("failed"));
        AWL_ATTRIBUTE(std::string, log_level, LogLevel::Trace);
        AWL_ATTRIBUTE(std::optional<std::string>, log_file, std::nullopt);
        AWL_ATTRIBUTE(std::string, run, {});

        if (!isLogLevel(log_level))
        {
            throw TestException(std::format("Not a valid 'log_level' parameter value: '{}'.", log_level));
        }

        bool passed = false;

        ostringstream last_output;
        _logger = makeTestConsoleLogger(output, log_level, log_file, last_output);
        context.logger = _logger;

        try
        {
            TestRunner runner(last_output);

            if (run.empty())
            {
                AWL_ATTRIBUTE(std::string, filter, ".*_Test");

                StaticMap<TestFunc> test_map{ StaticMap<TestFunc>::fill(filter) };

                context.logger->info("Running {} tests.", test_map.size());

                for (const TestLink* p_link : test_map)
                {
                    runner.runLink(p_link, context);
                }
            }
            else
            {
                context.logger->info("Running test {}.", run);

                const TestLink* p_link = static_chain<TestFunc>().find(run.c_str());

                if (p_link == nullptr)
                {
                    throw TestException(std::format(_T("The test '{}' does not exist."), run));
                }

                runner.runLink(p_link, context);
            }

            context.logger->info("The tests passed.");

            passed = true;
        }
        catch (const awl::testing::TestException& e)
        {
            context.logger->error(_T("The tests failed: {}"), e.message());

            if (output == _T("failed") && !last_output.str().empty())
            {
                awl::cout() << std::endl << last_output.str();
            }
        }

        // awl::static_chain<TestFunc>().clear();

        return passed;
    }

    template <attribute_provider Provider>
    int TestConsole<Provider>::run()
    {
        try
        {
            const bool passed = runTests();

            return passed ? 0 : 1;
        }
        catch (const TestException& e)
        {
            _logger->error(_T("The following error has occurred: {}"), e.message());
        }

        return 2;
    }

    int run(int argc, CmdChar* argv[], std::stop_token stop_token)
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
                auto logger = makeStdoutLogger(LogLevel::Trace);

                for (auto& p_link : test_map)
                {
                    logger->info("{}", p_link->name());
                }

                logger->info("Total {} tests.", test_map.size());

                return 0;
            }
        }

#ifdef AWL_QT

        QJsonObject jo;

        CmdString json_file;

        if (cl.tryGet("json", json_file))
        {
            try
            {
                jo = loadObjectFromFile(toQString(json_file));
            }
            catch (const JsonException& e)
            {
                makeStdoutLogger(LogLevel::Trace)->error(e.message());

                return 3;
            }
        }

        CompositeProvider<CommandLineProvider, JsonProvider> ap(std::move(cl), JsonProvider(jo));

#else

        CompositeProvider<CommandLineProvider> ap(std::move(cl));

#endif

        TestConsole console(ap, std::move(stop_token));

        auto guard = make_scope_guard([&ap, logger = console.context().logger]
        {
            auto names = ap.get_provider<0>().getUnusedOptions();

            for (const std::string& name : names)
            {
                logger->warning("Unused option '{}'.", name);
            }
        });

        return console.run();
    }

    int run(int argc, CmdChar* argv[])
    {
        std::stop_source source;

        return run(argc, argv, source.get_token());
    }

    int run()
    {
        std::stop_source source;

        return run(source.get_token());
    }

    int run(std::stop_token stop_token)
    {
        return run(0, nullptr, std::move(stop_token));
    }
}
