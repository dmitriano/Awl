#include "Awl/ConsoleLogger.h"

#include "Awl/String.h"
#include "Awl/Time.h"

#include <chrono>
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
    std::string_view fileName(const char* path)
    {
        const std::string_view full_path(path);
        const size_t pos = full_path.find_last_of("/\\");
        return pos == std::string_view::npos ? full_path : full_path.substr(pos + 1);
    }

    std::string joinSource(const std::vector<std::string>& source)
    {
        std::string result;

        for (const std::string& segment : source)
        {
            if (segment.empty())
            {
                continue;
            }

            if (!result.empty())
            {
                result.push_back('.');
            }

            result.append(segment);
        }

        return result;
    }
}

namespace awl
{
    ConsoleLogger::ConsoleLogger(
        awl::ostream& out,
        std::string min_level,
        std::string source) :
        _out(out),
        _minLevel(std::move(min_level)),
        _minSeverity(log_level_severity(_minLevel)),
        _source()
    {
        if (!source.empty())
        {
            _source.push_back(std::move(source));
        }
    }

    ConsoleLogger::ConsoleLogger(
        awl::ostream& out,
        std::string min_level,
        std::vector<std::string> source) :
        _out(out),
        _minLevel(std::move(min_level)),
        _minSeverity(log_level_severity(_minLevel)),
        _source(std::move(source))
    {}

    bool ConsoleLogger::enabled(const std::string& level) const
    {
        return log_level_severity(level) >= _minSeverity;
    }

    void ConsoleLogger::log(const std::string& level, const LogString& message)
    {
        awl::ostringstream temp_out;
        const std::source_location location = message.location();

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

        temp_out << _T('\t') << level;

        const std::string source = joinSource(_source);

        if (!source.empty())
        {
            temp_out << _T('\t') << awl::fromAString(source);
        }

        temp_out << _T('\t')
            << awl::fromAString(std::string(fileName(location.file_name())))
            << _T(':')
            << location.line()
            << _T('\t')
            << message.str()
            << _T('\n');

        _out << temp_out.str();
    }

    std::shared_ptr<ILogger> ConsoleLogger::createLogger(std::string source) const
    {
        std::vector<std::string> child_source = _source;

        if (!source.empty())
        {
            child_source.push_back(std::move(source));
        }

        return std::shared_ptr<ILogger>(new ConsoleLogger(
            _out,
            _minLevel,
            std::move(child_source)));
    }
}
