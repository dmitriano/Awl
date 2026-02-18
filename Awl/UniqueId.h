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
    using Id = std::uint64_t;

    [[nodiscard]] inline std::uint64_t unique_id() noexcept
    {
        static std::atomic<std::uint64_t> next_id{ 1 };

        const std::uint64_t id = next_id.fetch_add(1, std::memory_order_relaxed);

        if (id == 0)
        {
            // Wrap-around to 0 for uint64_t requires about 584 years even at 1e9 IDs/sec.
            std::terminate();
        }

        return id;
    }
}
