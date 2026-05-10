#include "Awl/StdStreamLogger.h"
#include "Awl/String.h"
#include "Awl/Time.h"

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
    struct StdStreamLogger::StreamState
    {
        explicit StreamState(std::shared_ptr<awl::ostream> stream) :
            out(std::move(stream))
        {}

        std::shared_ptr<awl::ostream> out;
        awl::ostringstream delayedOutput;
        bool delayed = false;
    };

    StdStreamLogger::StdStreamLogger(
        std::string source,
        std::shared_ptr<awl::ostream> out,
        std::string level,
        bool allow_custom_level) :
        _state(std::make_shared<StreamState>(std::move(out))),
        _level(std::move(level)),
        _severity(logLevelSeverity(_level)),
        _allowCustomLevel(allow_custom_level),
        _source()
    {
        if (!source.empty())
        {
            _source.push_back(std::move(source));
        }
    }

    StdStreamLogger::StdStreamLogger(
        std::vector<std::string> source,
        std::shared_ptr<StreamState> state,
        std::string level,
        bool allow_custom_level) :
        _state(std::move(state)),
        _level(std::move(level)),
        _severity(logLevelSeverity(_level)),
        _allowCustomLevel(allow_custom_level),
        _source(std::move(source))
    {}

    std::shared_ptr<awl::ostream> StdStreamLogger::wrapStream(awl::ostream& out)
    {
        return std::shared_ptr<awl::ostream>(&out, [](awl::ostream*) {});
    }

    std::shared_ptr<awl::ostream> StdStreamLogger::coutStream()
    {
        return wrapStream(awl::cout());
    }

    void StdStreamLogger::delay()
    {
        clearDelayed();
        _state->delayed = true;
    }

    void StdStreamLogger::flushDelayed()
    {
        _state->delayed = false;

        const String output = _state->delayedOutput.str();
        _state->delayedOutput.str(String());
        _state->delayedOutput.clear();

        if (!output.empty())
        {
            *_state->out << output;
            _state->out->flush();

            if (!*_state->out)
            {
                // Windows console streams can fail on valid Unicode market symbols.
                // See doc/fixes/2026-05-09-console-logger-unicode-stream-state.md.
                _state->out->clear();
            }
        }
    }

    void StdStreamLogger::clearDelayed()
    {
        _state->delayed = false;
        _state->delayedOutput.str(String());
        _state->delayedOutput.clear();
    }

    bool StdStreamLogger::enabled(const std::string& level) const
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

    void StdStreamLogger::doLog(const std::string& level, const LogString& message)
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

        if (_state->delayed)
        {
            _state->delayedOutput << temp_out.str();
            return;
        }

        *_state->out << temp_out.str();
        _state->out->flush();

        if (!*_state->out)
        {
            // Windows console streams can fail on valid Unicode market symbols.
            // See doc/fixes/2026-05-09-console-logger-unicode-stream-state.md.
            _state->out->clear();
        }
    }

    void StdStreamLogger::printSource(awl::ostream& out) const
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

    std::shared_ptr<ILogger> StdStreamLogger::createLogger(std::string source) const
    {
        if (source.empty())
        {
            return std::const_pointer_cast<ILogger>(shared_from_this());
        }
        else
        {
            std::vector<std::string> child_source = _source;
            child_source.push_back(std::move(source));

            return std::shared_ptr<ILogger>(new StdStreamLogger(
                std::move(child_source),
                _state,
                _level,
                _allowCustomLevel));
        }
    }
}
