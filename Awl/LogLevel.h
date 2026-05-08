/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/EnumTraits.h"
#include "Awl/String.h"

#include <algorithm>
#include <utility>

namespace awl
{
    AWL_SEQUENTIAL_ENUM(KnownLogLevel,
        Trace,
        Debug,
        Info,
        Warning,
        Error,
        Critical,
        Off
    )
}

AWL_ENUM_TRAITS(awl, KnownLogLevel)

namespace awl
{
    struct LogLevel
    {
        static inline const std::string Trace = enum_to_string(KnownLogLevel::Trace);
        static inline const std::string Debug = enum_to_string(KnownLogLevel::Debug);
        static inline const std::string Info = enum_to_string(KnownLogLevel::Info);
        static inline const std::string Warning = enum_to_string(KnownLogLevel::Warning);
        static inline const std::string Error = enum_to_string(KnownLogLevel::Error);
        static inline const std::string Critical = enum_to_string(KnownLogLevel::Critical);
        static inline const std::string Off = enum_to_string(KnownLogLevel::Off);
    };

    inline std::size_t log_level_severity(std::string level)
    {
        auto& names = EnumTraits<KnownLogLevel>::names();

        auto i = std::find_if(names.begin(), names.end(),
            std::bind(StringInsensitiveEqual<char>(), level, std::placeholders::_1));

        return static_cast<std::size_t>(i - names.begin());
    }

    inline bool is_log_level(std::string level)
    {
        return log_level_severity(std::move(level)) < EnumTraits<KnownLogLevel>::count();
    }
}
