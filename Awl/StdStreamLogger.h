/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ILogger.h"

#include "Awl/StdConsole.h"

#include <memory>
#include <string>
#include <vector>
namespace awl
{
    class StdStreamLogger : public ILogger
    {
    public:

        explicit StdStreamLogger(
            std::string source,
            std::shared_ptr<awl::ostream> out,
            std::string level = LogLevel::Trace,
            bool allow_custom_level = false);

        static std::shared_ptr<awl::ostream> wrapStream(awl::ostream& out);

        static std::shared_ptr<awl::ostream> coutStream();

        void delay();

        void flushDelayed();

        void clearDelayed();

        bool enabled(const std::string& level) const override;

        std::shared_ptr<ILogger> createLogger(std::string source) const override;

    protected:

        void doLog(const std::string& level, const LogString& message) override;

    private:

        struct StreamState;

        StdStreamLogger(
            std::vector<std::string> source,
            std::shared_ptr<StreamState> state,
            std::string level,
            bool allow_custom_level);

        void printSource(awl::ostream& out) const;

        std::shared_ptr<StreamState> _state;
        std::string _level;
        std::size_t _severity;
        bool _allowCustomLevel;
        std::vector<std::string> _source;
    };
}
