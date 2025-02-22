/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/LogString.h"
#include "Awl/EnumTraits.h"

#include <algorithm>

namespace awl
{
    AWL_SEQUENTIAL_ENUM(KnownLogLevel,
        Debug,
        Trace,
        Info,
        Warning,
        Error
    )
}

AWL_ENUM_TRAITS(awl, KnownLogLevel)

namespace awl
{
    struct LogLevel
    {
        static inline const std::string Debug = enum_to_string(KnownLogLevel::Debug);
        static inline const std::string Trace = enum_to_string(KnownLogLevel::Trace);
        static inline const std::string Info = enum_to_string(KnownLogLevel::Info);
        static inline const std::string Warning = enum_to_string(KnownLogLevel::Warning);
        static inline const std::string Error = enum_to_string(KnownLogLevel::Error);
    };

    inline std::size_t log_level_severity(std::string level)
    {
        auto& names = EnumTraits<KnownLogLevel>::names();

        auto i = std::find_if(names.begin(), names.end(),
            std::bind(StringInsensitiveEqual<char>(), level, std::placeholders::_1));

        return static_cast<std::size_t>(i - names.begin());
    }
}
