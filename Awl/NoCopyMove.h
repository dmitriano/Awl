/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

namespace awl
{
    class NoCopyMove
    {
    public:

        NoCopyMove(const NoCopyMove&) = delete;
        NoCopyMove& operator=(const NoCopyMove&) = delete;
        NoCopyMove(NoCopyMove&&) = delete;
        NoCopyMove& operator=(NoCopyMove&&) = delete;

    protected:

        NoCopyMove() = default;
        ~NoCopyMove() = default;
    };
}
