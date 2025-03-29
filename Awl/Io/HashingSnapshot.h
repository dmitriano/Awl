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
#include <cstdint>

namespace awl::io
{
    template <class OStream, class Hash>
    class HashingSnapshot : public Snapshot<OStream>
    {
    protected:

        using HashOStream = HashOutputStream<Hash, OStream>;

    private:

        using Value = std::vector<uint8_t>;

    public:

        HashingSnapshot(Value val, size_t block_size = defaultBlockSize, Hash hash = {}) :
            m_v(std::move(val)),
            m_blockSize(block_size),
            m_hash(hash)
        {}

        void Write(OStream& s) const override
        {
            HashOStream out{ s, m_blockSize, m_hash };

            // Write vector without leading 8 bytes containing its size.
            out.Write(m_v.data(), m_v.size());
        }

    private:

        Value m_v;
        size_t m_blockSize;
        Hash m_hash;
    };
}
