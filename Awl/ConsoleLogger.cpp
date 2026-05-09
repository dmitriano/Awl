#include "Awl/ConsoleLogger.h"

#include "Awl/String.h"
#include "Awl/Time.h"

#include <cassert>
#include <chrono>
#include <format>
#include <stdexcept>
#include <source_location>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#if defined(__APPLE__)
#include <ctime>
#endif

namespace
{
    std::string_view sourceFileName(const char* path)
    {
        const std::string_view full_path(path);
        const size_t pos = full_path.find_last_of("/\\");
        return pos == std::string_view::npos ? full_path : full_path.substr(pos + 1);
    }
}

namespace awl
{
    ConsoleLogger::ConsoleLogger(
        std::string source,
        awl::ostream& out,
        std::string level,
        bool allow_custom_level) :
        _out(out),
        _level(std::move(level)),
        _severity(logLevelSeverity(_level)),
        _allowCustomLevel(allow_custom_level),
        _source{ std::move(source) }
    {}

    ConsoleLogger::ConsoleLogger(
        std::vector<std::string> source,
        awl::ostream& out,
        std::string level,
        bool allow_custom_level) :
        _out(out),
        _level(std::move(level)),
        _severity(logLevelSeverity(_level)),
        _allowCustomLevel(allow_custom_level),
        _source(std::move(source))
    {}

    bool ConsoleLogger::enabled(const std::string& level) const
    {
        const std::size_t severity = logLevelSeverity(level);

        if (severity >= EnumTraits<KnownLogLevel>::count())
        {
            if (!_allowCustomLevel)
            {
                throw std::runtime_error(std::format("Unknown log level: '{}'.", level));
            }

            return true;
        }

        return severity >= _severity;
    }

    void ConsoleLogger::doLog(const std::string& level, const LogString& message)
    {
        awl::ostringstream temp_out;
        const std::source_location location = message.location();
        constexpr Char separator = _T('\t');

        // A quick fix to prevent compiler error "'to_chars' is unavailable: introduced in iOS 16.3"
        // with IPHONEOS_DEPLOYMENT_TARGET = 14.0 on Apple platform.
        // Alternatively IPHONEOS_DEPLOYMENT_TARGET can be set to 16.6, for example.
#if defined(__APPLE__)
        auto tp = std::chrono::system_clock::now();
        const std::time_t t = std::chrono::system_clock::to_time_t(tp);
        temp_out << std::ctime(&t);
#else
        temp_out << std::chrono::system_clock::now();
#endif

        temp_out << separator << level;

        if (!_source.empty())
        {
            temp_out << separator;
            printSource(temp_out);
        }

        temp_out << separator
            << awl::fromAString(std::string(sourceFileName(location.file_name())))
            << _T(':')
            << location.line()
            << separator
            << message.str()
            << _T('\n');

        _out << temp_out.str();
        _out.flush();

        if (!_out)
        {
            _out.clear();
        }
    }

    void ConsoleLogger::printSource(awl::ostream& out) const
    {
        bool first = true;

        for (const std::string& segment : _source)
        {
            if (!first)
            {
                out << _T('.');
            }

            out << awl::fromAString(segment);
            first = false;
        }
    }

    std::shared_ptr<ILogger> ConsoleLogger::createLogger(std::string source) const
    {
        assert(!source.empty());

        std::vector<std::string> child_source = _source;
        child_source.push_back(std::move(source));

        return std::shared_ptr<ILogger>(new ConsoleLogger(
            std::move(child_source),
            _out,
            _level,
            _allowCustomLevel));
    }
}
