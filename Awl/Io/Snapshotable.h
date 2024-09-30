/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include <cstdint>

namespace awl::io
{
    using Snapshot = std::vector<uint8_t>;

    template <class OStream>
    class Snapshotable
    {
    public:

        virtual Snapshot MakeShanshot() const = 0;

        virtual void WriteSnapshot(OStream& out, const Snapshot& v) = 0;

        virtual ~Snapshotable() = default;
    };
}
