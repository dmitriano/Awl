/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <memory>

namespace awl::io
{
    class Snapshot
    {
    public:

        // Snapshot is typically an std::vector<std::byte> so we write it to a basic stream.
        virtual void write(SequentialOutputStream& out) const = 0;

        virtual ~Snapshot() = default;
    };

    class Snapshotable
    {
    public:

        virtual std::shared_ptr<Snapshot> makeShanshot() const = 0;

        virtual ~Snapshotable() = default;
    };
}
