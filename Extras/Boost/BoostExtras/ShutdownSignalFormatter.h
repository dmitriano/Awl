#pragma once

#include "BoostExtras/ShutdownSignals.h"

#include "Awl/StringFormat.h"

#include <format>
#include <string>
#include <string_view>

namespace awl
{
    constexpr std::string_view shutdownSignalName(const ShutdownSignal signal) noexcept
    {
        switch (signal)
        {
        case ShutdownSignal::Interrupt:
            return "Ctrl+C (SIGINT)";
        case ShutdownSignal::Terminate:
            return "SIGTERM";
        case ShutdownSignal::Break:
            return "Ctrl+Break (SIGBREAK)";
        default:
            return "Unknown shutdown signal";
        }
    }
}

namespace std
{
    template <class CharT>
    struct formatter<awl::ShutdownSignal, CharT> : formatter<std::string, CharT>
    {
        template <class FormatContext>
        auto format(const awl::ShutdownSignal signal, FormatContext& ctx) const
        {
            return formatter<std::string, CharT>::format(
                std::string(awl::shutdownSignalName(signal)), ctx);
        }
    };
}
