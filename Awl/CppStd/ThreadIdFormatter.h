#pragma once

#ifdef AWL_THREAD_ID_FORMATTER

#include <thread>
#include <format>
#include <cstdint>
#include <functional>

template<>
struct std::formatter<std::thread::id> : std::formatter<std::size_t>
{
    auto format(const std::thread::id& id, std::format_context& ctx) const
    {
        const size_t value = std::hash<std::thread::id>{}(id);

        // Delegate formatting to base formatter (allows custom {:x}, {:08x}, etc.)
        return std::formatter<uintptr_t>::format(value, ctx);
    }
};

#endif
