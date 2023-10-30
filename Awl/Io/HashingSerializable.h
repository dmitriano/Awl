#pragma once

#include "Awl/Io/Serializable.h"
#include "Awl/Io/HashStream.h"
#include "Awl/Crypto/Crc64.h"

namespace awl::io
{
    template <class IStream = SequentialInputStream, class OStream = SequentialOutputStream, class Hash = awl::crypto::Crc64>
    class HashingSerializable : public Serializable<IStream, OStream>
    {
    private:

        using HashInputStream = HashInputStream<Hash, IStream>;
        using HashOutputStream = HashOutputStream<Hash, OStream>;

        using Value = Serializable<HashInputStream, HashOutputStream>;

    public:

        HashingSerializable(Value& val, size_t block_size = defaultBlockSize, Hash hash = {}) :
            m_val(val),
            m_blockSize(block_size),
            m_hash(hash)
        {}

        void Read(IStream& s) override
        {
            HashInputStream in(s, m_blockSize, m_hash);

            val.Read(in);
        }

        void Write(OStream& s) const override
        {
            HashOutputStream out(s, m_blockSize, m_hash);

            val.Write(out);
        }

    private:

        Value& m_val;
        size_t m_blockSize
        Hash m_hash;
    };
}
