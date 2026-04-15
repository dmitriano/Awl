#pragma once

#ifdef AWL_THREAD_ID_FORMATTER

#include <thread>
#include <format>
#include <cstdint>
#include <functional>

template<class CharT>
struct std::formatter<std::thread::id, CharT> : std::formatter<std::size_t, CharT>
{
    template <class FormatContext>
    auto format(const std::thread::id& id, FormatContext& ctx) const
    {
        const size_t value = std::hash<std::thread::id>{}(id);

        // Delegate formatting to base formatter (allows custom {:x}, {:08x}, etc.)
        return std::formatter<std::size_t, CharT>::format(value, ctx);
    }
};

#endif
