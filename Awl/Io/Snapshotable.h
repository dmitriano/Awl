/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <memory>

namespace awl::io
{
    template <class OStream = SequentialOutputStream>
    class Snapshot
    {
    public:

        virtual void Write(OStream& out) const = 0;

        virtual ~Snapshot() = default;
    };

    template <class OStream>
    class Snapshotable
    {
    public:

        virtual std::shared_ptr<Snapshot<OStream>> MakeShanshot() const = 0;

        virtual ~Snapshotable() = default;
    };
}
