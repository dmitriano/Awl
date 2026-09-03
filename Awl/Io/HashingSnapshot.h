/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/Snapshotable.h"
#include "Awl/Io/HashStream.h"
#include "Awl/Io/Rw/VectorReadWrite.h"
#include "Awl/Crypto/Crc64.h"

#include <vector>
#include <cstddef>

namespace awl::io
{
    template <class Hash>
    class HashingSnapshot : public Snapshot
    {
    protected:

        using HashOStream = HashOutputStream<Hash, SequentialOutputStream>;

    private:

        using Value = std::vector<std::byte>;

    public:

        HashingSnapshot(Value val, size_t block_size = defaultBlockSize, Hash hash = {}) :
            _v(std::move(val)),
            _blockSize(block_size),
            _hash(hash)
        {}

        void write(SequentialOutputStream& s) const override
        {
            HashOStream out{ s, _blockSize, _hash };

            // Write vector without leading 8 bytes containing its size.
            out.write(_v.data(), _v.size());
        }

    private:

        Value _v;
        size_t _blockSize;
        Hash _hash;
    };
}
