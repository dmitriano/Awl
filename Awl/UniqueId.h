/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <atomic>
#include <cstdint>
#include <exception>

namespace awl
{
    [[nodiscard]] inline std::uint64_t unique_id() noexcept
    {
        static std::atomic<std::uint64_t> next_id{ 1 };

        const std::uint64_t id = next_id.fetch_add(1, std::memory_order_relaxed);

        if (id == 0)
        {
            // 500 years has elapsed since the Unix epoch, so this is not expected to happen in practice.
            std::terminate();
        }

        return id;
    }
}
