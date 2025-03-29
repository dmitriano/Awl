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

        // Snapshot is typically an std::vector<uint8_t> so we wite it to a basic stream.
        virtual void Write(SequentialOutputStream& out) const = 0;

        virtual ~Snapshot() = default;
    };

    class Snapshotable
    {
    public:

        virtual std::shared_ptr<Snapshot> MakeShanshot() const = 0;

        virtual ~Snapshotable() = default;
    };
}
