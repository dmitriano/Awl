/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Awl/ConsoleLogger.h"
#include "Awl/StringFormat.h"

#include "Awl/Testing/UnitTest.h"

namespace
{
    struct CountedFormatValue
    {
        int* format_count = nullptr;
    };

    class CaptureLogger : public awl::Logger
    {
    public:

        using awl::Logger::log;

        void log(const std::string& level, const awl::LogString& message) override
        {
            ++m_log_count;
            m_level = level;

#ifdef AWL_QT
            m_message = awl::StringConvertor<awl::Char>::convertFrom(message.str().toStdString().c_str());
#else
            m_message = message.str();
#endif
        }

        const std::string& level() const
        {
            return m_level;
        }

        const awl::String& message() const
        {
            return m_message;
        }

        int logCount() const
        {
            return m_log_count;
        }

    private:

        int m_log_count = 0;
        std::string m_level;
        awl::String m_message;
    };

    class FilteredLogger : public CaptureLogger
    {
    public:

        bool enabled(const std::string& level) const override
        {
            return level != awl::LogLevel::Debug;
        }
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

AWL_TEST(Logger)
{
    // Check if it comiles with all the strings.
    context.logger.debug("abc");
    context.logger.debug(std::string("abc"));
    context.logger.debug("abc");

    context.logger.trace(L"abc");
    context.logger.trace(std::wstring(L"abc"));
    context.logger.trace(L"abc");

    context.logger.info(awl::String(_T("abc")));
    context.logger.info(_T("abc"));
    context.logger.info(_T("abc"));

#ifdef AWL_QT

    context.logger.warning(QString("abc"));

#endif

    CaptureLogger logger;

    logger.debug(_T("value={}, hex={:x}"), 42, 42);

    AWL_ASSERT_EQUAL(awl::LogLevel::Debug, logger.level());
    AWL_ASSERT_EQUAL(std::format(_T("value={}, hex={:x}"), 42, 42), logger.message());

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
}
