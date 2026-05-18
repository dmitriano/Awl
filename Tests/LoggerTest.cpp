/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/CompositeLogger.h"
#include "Awl/StdStreamLogger.h"
#include "Awl/StringFormat.h"
#include "Awl/Testing/UnitTest.h"

#include <memory>
#include <source_location>
#include <stdexcept>
#include <string_view>
#include <vector>

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

        CaptureLogger() = default;

        explicit CaptureLogger(std::string source) :
            _source(std::move(source))
        {}

        std::shared_ptr<awl::ILogger> createLogger(std::string source) const override
        {
            auto child = std::make_shared<CaptureLogger>(std::move(source));
            _children.push_back(child);
            return child;
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

        const std::string& source() const
        {
            return _source;
        }

        const std::shared_ptr<CaptureLogger>& child(size_t index) const
        {
            return _children[index];
        }

    protected:

        void doLog(const std::string& level, const awl::LogString& message) noexcept override
        {
            ++_logCount;
            _level = level;
            _location = message.location();
            _message = message.str();
        }

    private:

        std::string _source;
        mutable std::vector<std::shared_ptr<CaptureLogger>> _children;
        int _logCount = 0;
        std::string _level;
        awl::String _message;
        std::source_location _location;
    };

    class FilteredLogger : public CaptureLogger
    {
    public:

        bool enabled(const std::string& level) const noexcept override
        {
            return level != awl::LogLevel::Debug;
        }
    };

    class CompositeTestLogger : public awl::ILogger
    {
    public:

        explicit CompositeTestLogger(bool enabled) :
            _enabled(enabled)
        {}

        bool enabled(const std::string& level) const noexcept override
        {
            ++_enabledCallCount;
            _enabledLevel = level;
            return _enabled;
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

    protected:

        void doLog(const std::string& level, const awl::LogString& message) noexcept override
        {
            ++_logCount;
            _level = level;
            _message = message.str();
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
    auto out_stream = awl::StdStreamLogger::wrapStream(out);
    awl::StdStreamLogger default_logger("Test", out_stream);

    AWL_ASSERT(default_logger.enabled(awl::LogLevel::Trace));
    AWL_ASSERT_FALSE(default_logger.enabled("Custom"));

    awl::StdStreamLogger console_logger("Test", out_stream, awl::LogLevel::Info);

    AWL_ASSERT_FALSE(console_logger.enabled(awl::LogLevel::Debug));
    AWL_ASSERT_FALSE(console_logger.enabled(awl::LogLevel::Trace));
    AWL_ASSERT(console_logger.enabled(awl::LogLevel::Info));
    AWL_ASSERT(console_logger.enabled(awl::LogLevel::Warning));
    AWL_ASSERT(console_logger.enabled(awl::LogLevel::Error));
    AWL_ASSERT(console_logger.enabled(awl::LogLevel::Critical));

    awl::StdStreamLogger debug_logger("Test", out_stream, awl::LogLevel::Debug);

    AWL_ASSERT_FALSE(debug_logger.enabled(awl::LogLevel::Trace));
    AWL_ASSERT(debug_logger.enabled(awl::LogLevel::Debug));

    awl::StdStreamLogger critical_logger("Test", out_stream, awl::LogLevel::Critical);

    AWL_ASSERT_FALSE(critical_logger.enabled(awl::LogLevel::Error));
    AWL_ASSERT(critical_logger.enabled(awl::LogLevel::Critical));

    awl::StdStreamLogger off_logger("Test", out_stream, awl::LogLevel::Off);

    AWL_ASSERT_FALSE(off_logger.enabled(awl::LogLevel::Error));
    AWL_ASSERT_FALSE(off_logger.enabled(awl::LogLevel::Critical));

    awl::StdStreamLogger custom_logger("Test", out_stream, awl::LogLevel::Info, true);

    AWL_ASSERT(custom_logger.enabled("Custom"));
    AWL_ASSERT(custom_logger.createLogger("Child")->enabled("Custom"));

    awl::StdStreamLogger custom_off_logger("Test", out_stream, awl::LogLevel::Off, true);

    AWL_ASSERT(custom_off_logger.enabled("Custom"));

    auto root_logger = std::make_shared<awl::StdStreamLogger>("Root", out_stream, awl::LogLevel::Info);
    std::shared_ptr<awl::ILogger> child_logger = root_logger->createLogger("Child");
    child_logger->info("source message");

    AWL_ASSERT(out.str().find(_T("Root.Child")) != awl::String::npos);
}

AWL_TEST(CompositeLogger)
{
    AWL_UNUSED_CONTEXT;

    auto disabled_logger = std::make_shared<CompositeTestLogger>(false);
    auto enabled_logger = std::make_shared<CompositeTestLogger>(true);
    auto skipped_logger = std::make_shared<CompositeTestLogger>(true);
    awl::CompositeLogger logger;

    AWL_ASSERT_FALSE(logger.enabled(awl::LogLevel::Info));

    logger.addLogger(disabled_logger);
    logger.addLogger(enabled_logger);
    logger.addLogger(skipped_logger);

    AWL_ASSERT(logger.enabled(awl::LogLevel::Warning));
    AWL_ASSERT_EQUAL(1, disabled_logger->enabledCallCount());
    AWL_ASSERT_EQUAL(1, enabled_logger->enabledCallCount());
    AWL_ASSERT_EQUAL(0, skipped_logger->enabledCallCount());
    AWL_ASSERT_EQUAL(awl::LogLevel::Warning, disabled_logger->enabledLevel());
    AWL_ASSERT_EQUAL(awl::LogLevel::Warning, enabled_logger->enabledLevel());

    logger.error("composite {}", 42);

    AWL_ASSERT_EQUAL(0, disabled_logger->logCount());
    AWL_ASSERT_EQUAL(1, enabled_logger->logCount());
    AWL_ASSERT_EQUAL(1, skipped_logger->logCount());
    AWL_ASSERT_EQUAL(awl::LogLevel::Error, enabled_logger->level());
    AWL_ASSERT_EQUAL(_T("composite 42"), enabled_logger->message());

    AWL_ASSERT(logger.removeLogger(enabled_logger));
    AWL_ASSERT_FALSE(logger.removeLogger(enabled_logger));

    logger.error("after remove");

    AWL_ASSERT_EQUAL(1, enabled_logger->logCount());
    AWL_ASSERT_EQUAL(2, skipped_logger->logCount());
    AWL_ASSERT_EQUAL(_T("after remove"), skipped_logger->message());

    auto capture_logger = std::make_shared<CaptureLogger>();
    awl::CompositeLogger parent_logger({ capture_logger });
    std::shared_ptr<awl::ILogger> child_logger = parent_logger.createLogger("Child");

    AWL_ASSERT(child_logger != nullptr);

    child_logger->info("child message");

    AWL_ASSERT_EQUAL(0, capture_logger->logCount());
    AWL_ASSERT_EQUAL("Child", capture_logger->child(0)->source());
    AWL_ASSERT_EQUAL(1, capture_logger->child(0)->logCount());
    AWL_ASSERT_EQUAL(_T("child message"), capture_logger->child(0)->message());
}
