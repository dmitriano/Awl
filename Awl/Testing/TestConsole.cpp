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
#include "Awl/EnumTraits.h"
#include "Awl/IntRange.h"
#include "Awl/ScopeGuard.h"
#include "Awl/StaticMap.h"
#include "Awl/StdStreamLogger.h"

#ifdef AWL_BOOST
    #include "BoostExtras/Json/JsonUtil.h"
#endif

#ifdef AWL_QT
    #include "QtExtras/StringConversion.h"
#endif

#include <filesystem>
#include <fstream>
#include <iterator>
#include <optional>
#include <set>
#include <regex>
#include <vector>

namespace awl::testing
{
    AWL_SEQUENTIAL_ENUM(TestOutput, Failed, All, Null)
}

AWL_ENUM_TRAITS(awl::testing, TestOutput)

namespace awl::testing
{
    namespace
    {
        struct TestConsoleLogger
        {
            std::shared_ptr<CompositeLogger> logger;
            std::vector<std::shared_ptr<StdStreamLogger>> delayedLoggers;
        };

        std::shared_ptr<CompositeLogger> makeStdoutLogger(const std::string& log_level)
        {
            std::shared_ptr<CompositeLogger> logger = std::make_shared<CompositeLogger>();
            logger->addLogger(std::make_shared<StdStreamLogger>(
                "",
                StdStreamLogger::coutStream(),
                log_level));
            return logger;
        }

        void addFileLogger(
            TestConsoleLogger& console_logger,
            const std::optional<std::string>& log_file,
            const std::string& log_level,
            const std::optional<std::string>& file_level,
            bool delayed)
        {
            if (!log_file)
            {
                return;
            }

            std::shared_ptr<std::basic_ofstream<Char>> file_out = std::make_shared<std::basic_ofstream<Char>>(
                std::filesystem::path(*log_file),
                std::ios_base::trunc);

            if (!file_out->is_open())
            {
                throw TestException(std::format("Cannot open log file '{}'.", *log_file));
            }

            const std::string& effective_log_level = file_level ? *file_level : log_level;

            std::shared_ptr<StdStreamLogger> logger = std::make_shared<StdStreamLogger>(
                "",
                file_out,
                effective_log_level);

            console_logger.logger->addLogger(logger);

            if (delayed)
            {
                console_logger.delayedLoggers.push_back(logger);
            }
        }

        void addStdoutLogger(
            TestConsoleLogger& console_logger,
            const std::string& log_level,
            bool delayed)
        {
            std::shared_ptr<StdStreamLogger> logger = std::make_shared<StdStreamLogger>(
                "",
                StdStreamLogger::coutStream(),
                log_level);

            console_logger.logger->addLogger(logger);

            if (delayed)
            {
                console_logger.delayedLoggers.push_back(logger);
            }
        }

        void delayLoggers(const std::vector<std::shared_ptr<StdStreamLogger>>& loggers)
        {
            for (const std::shared_ptr<StdStreamLogger>& logger : loggers)
            {
                logger->delay();
            }
        }

        void flushDelayedLoggers(const std::vector<std::shared_ptr<StdStreamLogger>>& loggers)
        {
            for (const std::shared_ptr<StdStreamLogger>& logger : loggers)
            {
                logger->flushDelayed();
            }
        }

        void clearDelayedLoggers(const std::vector<std::shared_ptr<StdStreamLogger>>& loggers)
        {
            for (const std::shared_ptr<StdStreamLogger>& logger : loggers)
            {
                logger->clearDelayed();
            }
        }

        TestConsoleLogger makeTestConsoleLogger(
            TestOutput output,
            const std::string& log_level,
            const std::optional<std::string>& file_level,
            const std::optional<std::string>& log_file)
        {
            TestConsoleLogger console_logger{ std::make_shared<CompositeLogger>(), {} };

            switch (output)
            {
            case TestOutput::All:
                addStdoutLogger(console_logger, log_level, false);
                break;

            case TestOutput::Failed:
                addStdoutLogger(console_logger, log_level, true);
                break;

            case TestOutput::Null:
                break;
            }

            addFileLogger(console_logger, log_file, log_level, file_level, output == TestOutput::Failed);

            return console_logger;
        }

#ifdef AWL_BOOST

        boost::json::object loadTestAttributes(const CmdString& file_name)
        {
            std::ifstream in(std::filesystem::path(file_name), std::ios_base::binary);

            if (!in.is_open())
            {
                throw JsonException(std::format(_T("Cannot open input file '{}'."), file_name));
            }

            const std::string text{
                std::istreambuf_iterator<char>(in),
                std::istreambuf_iterator<char>()};

            boost::json::value jv = boost::json::parse(text);

            if (!jv.is_object())
            {
                throw JsonException(std::format(_T("JSON object expected in file '{}'."), file_name));
            }

            return jv.as_object();
        }

#endif
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
        
        AWL_ATTRIBUTE(TestOutput, output, TestOutput::Failed);
        AWL_ATTRIBUTE(std::string, log_level, LogLevel::Trace);
        AWL_ATTRIBUTE(std::optional<std::string>, file_level, std::nullopt);
        AWL_ATTRIBUTE(std::optional<std::string>, log_file, std::nullopt);
        AWL_ATTRIBUTE(std::string, run, {});

        if (!isLogLevel(log_level))
        {
            throw TestException(std::format("Not a valid 'log_level' parameter value: '{}'.", log_level));
        }

        if (file_level && !isLogLevel(*file_level))
        {
            throw TestException(std::format("Not a valid 'file_level' parameter value: '{}'.", *file_level));
        }

        bool passed = false;

        TestConsoleLogger console_logger = makeTestConsoleLogger(output, log_level, file_level, log_file);
        _logger = console_logger.logger;
        context.logger = _logger;

        try
        {
            TestRunner runner(
                [&console_logger]
                {
                    delayLoggers(console_logger.delayedLoggers);
                },
                [&console_logger]
                {
                    clearDelayedLoggers(console_logger.delayedLoggers);
                });

            if (run.empty())
            {
                AWL_ATTRIBUTE(std::string, filter, ".*_Test");

                StaticMap<TestFunc> test_map{ StaticMap<TestFunc>::fill(filter) };

                context.logger->info("Running {} tests.", test_map.size());

                for (const TestLink* p_link : test_map)
                {
                    runner.runLink(p_link, context, context.logger->createLogger(p_link->name()));
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

                runner.runLink(p_link, context, context.logger);
            }

            context.logger->info("The tests passed.");

            passed = true;
        }
        catch (const awl::testing::TestException& e)
        {
            flushDelayedLoggers(console_logger.delayedLoggers);
            context.logger->error(_T("The tests failed: {}"), e.message());
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
        try
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

#ifdef AWL_BOOST

            boost::json::object jo;

            CmdString json_file;

            if (cl.tryGet("json", json_file))
            {
                try
                {
                    jo = loadTestAttributes(json_file);
                }
                catch (const JsonException& e)
                {
                    makeStdoutLogger(LogLevel::Trace)->error(e.message());

                    return 3;
                }
            }

            CompositeProvider<CommandLineProvider, JsonProvider> ap(std::move(cl), JsonProvider(std::move(jo)));

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
        catch (const CommandLineException& e)
        {
            makeStdoutLogger(LogLevel::Trace)->error(_T("The following error has occurred: {}"), e.message());
        }

        return 2;
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
