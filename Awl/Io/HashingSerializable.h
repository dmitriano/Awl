/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/Serializable.h"
#include "Awl/Io/HashStream.h"
#include "Awl/Io/HashingSnapshot.h"
#include "Awl/Io/Rw/VectorReadWrite.h"
#include "Awl/Crypto/Crc64.h"

#include <vector>
#include <memory>
#include <cstdint>

namespace awl::io
{
    template <class IStream = SequentialInputStream, class OStream = SequentialOutputStream, class Hash = awl::crypto::Crc64>
    class HashingSerializable :
        public Serializable<IStream, OStream>
    {
    protected:

        using HashIStream = HashInputStream<Hash, IStream>;
        using HashOStream = HashOutputStream<Hash, OStream>;

    private:

        using Value = Serializable<HashIStream, HashOStream>;

    public:

        HashingSerializable(Value& val, size_t block_size = defaultBlockSize, Hash hash = {}) :
            m_val(val),
            m_blockSize(block_size),
            m_hash(hash)
        {}

        void read(IStream& s) override
        {
            HashIStream in{ s, m_blockSize, m_hash };

            if (readHeader(in))
            {
                m_val.read(in);
            }
        }

        void write(OStream& s) const override
        {
            HashOStream out{ s, m_blockSize, m_hash };

            writeHeader(out);

            m_val.write(out);
        }

    protected:

        virtual bool readHeader(awl::io::SequentialInputStream&) { return true; }

        virtual void writeHeader(awl::io::SequentialOutputStream&) const {}

        std::shared_ptr<Snapshot> makeShanshotHelper(std::vector<uint8_t> v) const
        {
            return std::make_shared<HashingSnapshot<Hash>>(std::move(v), m_blockSize, m_hash);
        }

    private:

        Value& m_val;
        size_t m_blockSize;
        Hash m_hash;
    };
}
