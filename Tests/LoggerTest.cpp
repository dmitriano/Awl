/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/CompositeLogger.h"
#include "Awl/ConsoleLogger.h"
#include "Awl/LegacyFormat.h"
#include "Awl/StringFormat.h"

#include "Awl/Testing/UnitTest.h"

#include <memory>
#include <source_location>
#include <string_view>

namespace
{
    std::string_view fileName(const char* path)
    {
        const std::string_view full_path(path);
        const size_t pos = full_path.find_last_of("/\\");
        return pos == std::string_view::npos ? full_path : full_path.substr(pos + 1);
    }

    struct CountedFormatValue
    {
        int* format_count = nullptr;
    };

    class CaptureLogger : public awl::ILogger
    {
    public:

        using awl::ILogger::log;

        void log(const std::string& level, const awl::LogString& message) override
        {
            ++_logCount;
            _level = level;
            _location = message.location();
            _message = message.str();
        }

        std::shared_ptr<awl::ILogger> createLogger(std::string source) const override
        {
            static_cast<void>(source);
            return std::make_shared<CaptureLogger>();
        }

        const std::string& level() const
        {
            return _level;
        }

        const awl::String& message() const
        {
            return _message;
        }

        std::source_location location() const
        {
            return _location;
        }

        int logCount() const
        {
            return _logCount;
        }

    private:

        int _logCount = 0;
        std::string _level;
        awl::String _message;
        std::source_location _location;
    };

    class FilteredLogger : public CaptureLogger
    {
    public:

        bool enabled(const std::string& level) const override
        {
            return level != awl::LogLevel::Debug;
        }
    };

    class CompositeTestLogger : public awl::Observer<awl::ILogger>
    {
    public:

        using awl::ILogger::log;

        explicit CompositeTestLogger(bool enabled) :
            _enabled(enabled)
        {}

        bool enabled(const std::string& level) const override
        {
            ++_enabledCallCount;
            _enabledLevel = level;
            return _enabled;
        }

        void log(const std::string& level, const awl::LogString& message) override
        {
            ++_logCount;
            _level = level;
            _message = message.str();
        }

        std::shared_ptr<awl::ILogger> createLogger(std::string source) const override
        {
            static_cast<void>(source);
            return nullptr;
        }

        int enabledCallCount() const
        {
            return _enabledCallCount;
        }

        int logCount() const
        {
            return _logCount;
        }

        const std::string& enabledLevel() const
        {
            return _enabledLevel;
        }

        const std::string& level() const
        {
            return _level;
        }

        const awl::String& message() const
        {
            return _message;
        }

    private:

        bool _enabled;
        mutable int _enabledCallCount = 0;
        mutable std::string _enabledLevel;
        int _logCount = 0;
        std::string _level;
        awl::String _message;
    };
}

namespace std
{
    template <class CharT>
    struct formatter<CountedFormatValue, CharT> : formatter<int, CharT>
    {
        template <class FormatContext>
        auto format(const CountedFormatValue& val, FormatContext& ctx) const
        {
            ++(*val.format_count);
            return formatter<int, CharT>::format(0, ctx);
        }
    };
}

AWL_TEST(ILogger)
{
    // Check if it comiles with all the strings.
    context.logger->debug("abc");
    context.logger->debug(std::string("abc"));
    context.logger->debug("abc");

    context.logger->trace(L"abc");
    context.logger->trace(std::wstring(L"abc"));
    context.logger->trace(L"abc");

    context.logger->info(awl::String(_T("abc")));
    context.logger->info(_T("abc"));
    context.logger->info(_T("abc"));
    context.logger->critical("abc");

    CaptureLogger logger;

    const auto expected_debug_location = std::source_location::current();
    logger.debug(_T("value={}, hex={:x}"), 42, 42);

    AWL_ASSERT_EQUAL(awl::LogLevel::Debug, logger.level());
    AWL_ASSERT_EQUAL(std::format(_T("value={}, hex={:x}"), 42, 42), logger.message());
    AWL_ASSERT_EQUAL(expected_debug_location.line() + 1, logger.location().line());
    AWL_ASSERT(fileName(expected_debug_location.file_name()) == fileName(logger.location().file_name()));

    const auto expected_legacy_location = std::source_location::current();
    awl::LogString legacy_message = awl::format() << "legacy " << 11;

    AWL_ASSERT_EQUAL(_T("legacy 11"), legacy_message.str());
    AWL_ASSERT_EQUAL(expected_legacy_location.line() + 1, legacy_message.location().line());
    AWL_ASSERT(fileName(expected_legacy_location.file_name()) == fileName(legacy_message.location().file_name()));

    logger.error(_T("wide {}"), awl::String(_T("message")));

    AWL_ASSERT_EQUAL(awl::LogLevel::Error, logger.level());
    AWL_ASSERT_EQUAL(std::format(_T("wide {}"), awl::String(_T("message"))), logger.message());

    logger.log(awl::LogLevel::Info, _T("direct {}"), 7);

    AWL_ASSERT_EQUAL(awl::LogLevel::Info, logger.level());
    AWL_ASSERT_EQUAL(std::format(_T("direct {}"), 7), logger.message());

    FilteredLogger filtered_logger;
    const int initial_log_count = filtered_logger.logCount();

    int format_count = 0;
    filtered_logger.debug(_T("disabled {}"), CountedFormatValue{ &format_count });

    AWL_ASSERT_EQUAL(0, format_count);
    AWL_ASSERT_EQUAL(initial_log_count, filtered_logger.logCount());

    awl::ostringstream out;
    awl::ConsoleLogger console_logger(out, awl::LogLevel::Info);

    AWL_ASSERT_FALSE(console_logger.enabled(awl::LogLevel::Debug));
    AWL_ASSERT_FALSE(console_logger.enabled(awl::LogLevel::Trace));
    AWL_ASSERT(console_logger.enabled(awl::LogLevel::Info));
    AWL_ASSERT(console_logger.enabled(awl::LogLevel::Warning));
    AWL_ASSERT(console_logger.enabled(awl::LogLevel::Error));
    AWL_ASSERT(console_logger.enabled(awl::LogLevel::Critical));

    awl::ConsoleLogger debug_logger(out, awl::LogLevel::Debug);

    AWL_ASSERT_FALSE(debug_logger.enabled(awl::LogLevel::Trace));
    AWL_ASSERT(debug_logger.enabled(awl::LogLevel::Debug));

    awl::ConsoleLogger critical_logger(out, awl::LogLevel::Critical);

    AWL_ASSERT_FALSE(critical_logger.enabled(awl::LogLevel::Error));
    AWL_ASSERT(critical_logger.enabled(awl::LogLevel::Critical));

    awl::ConsoleLogger off_logger(out, awl::LogLevel::Off);

    AWL_ASSERT_FALSE(off_logger.enabled(awl::LogLevel::Error));
    AWL_ASSERT_FALSE(off_logger.enabled(awl::LogLevel::Critical));

    auto root_logger = std::make_shared<awl::ConsoleLogger>(out, awl::LogLevel::Info, "Root");
    std::shared_ptr<awl::ILogger> child_logger = root_logger->createLogger("Child");
    child_logger->info("source message");

    AWL_ASSERT(out.str().find(_T("Root.Child")) != awl::String::npos);
}

AWL_TEST(CompositeLogger)
{
    AWL_UNUSED_CONTEXT;

    CompositeTestLogger disabled_logger(false);
    CompositeTestLogger enabled_logger(true);
    CompositeTestLogger skipped_logger(true);
    awl::CompositeLogger logger;

    AWL_ASSERT_FALSE(logger.enabled(awl::LogLevel::Info));

    logger.subscribe(&disabled_logger);
    logger.subscribe(&enabled_logger);
    logger.subscribe(&skipped_logger);

    AWL_ASSERT(logger.enabled(awl::LogLevel::Warning));
    AWL_ASSERT_EQUAL(1, disabled_logger.enabledCallCount());
    AWL_ASSERT_EQUAL(1, enabled_logger.enabledCallCount());
    AWL_ASSERT_EQUAL(0, skipped_logger.enabledCallCount());
    AWL_ASSERT_EQUAL(awl::LogLevel::Warning, disabled_logger.enabledLevel());
    AWL_ASSERT_EQUAL(awl::LogLevel::Warning, enabled_logger.enabledLevel());

    logger.error("composite {}", 42);

    AWL_ASSERT_EQUAL(1, disabled_logger.logCount());
    AWL_ASSERT_EQUAL(1, enabled_logger.logCount());
    AWL_ASSERT_EQUAL(1, skipped_logger.logCount());
    AWL_ASSERT_EQUAL(awl::LogLevel::Error, disabled_logger.level());
    AWL_ASSERT_EQUAL(_T("composite 42"), disabled_logger.message());
    AWL_ASSERT_EQUAL(awl::LogLevel::Error, enabled_logger.level());
    AWL_ASSERT_EQUAL(_T("composite 42"), enabled_logger.message());

    logger.unsubscribe(&enabled_logger);

    logger.info("after unsubscribe");

    AWL_ASSERT_EQUAL(2, disabled_logger.logCount());
    AWL_ASSERT_EQUAL(1, enabled_logger.logCount());
    AWL_ASSERT_EQUAL(2, skipped_logger.logCount());

    AWL_ASSERT(logger.createLogger("Child") == nullptr);
}
