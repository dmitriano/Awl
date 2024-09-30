/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/Serializable.h"
#include "Awl/Io/HashStream.h"
#include "Awl/Io/Rw/VectorReadWrite.h"
#include "Awl/Crypto/Crc64.h"

#include <vector>
#include <cstdint>

namespace awl::io
{
    template <class IStream = SequentialInputStream, class OStream = SequentialOutputStream, class Hash = awl::crypto::Crc64>
    class HashingSerializable : public Serializable<IStream, OStream>
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

        void Read(IStream& s) override
        {
            HashIStream in(s, m_blockSize, m_hash);

            if (ReadHeader(in))
            {
                m_val.Read(in);
            }
        }

        void Write(OStream& s) const override
        {
            HashOStream out = MakeHashingOutputStream(s);

            WriteHeader(out);

            m_val.Write(out);
        }

    protected:

        virtual bool ReadHeader(HashIStream&) { return true; }

        virtual void WriteHeader(HashOStream&) const {}

        HashOStream MakeHashingOutputStream(OStream& out) const
        {
            return { out, m_blockSize, m_hash };
        }

    private:

        Value& m_val;
        size_t m_blockSize;
        Hash m_hash;
    };
}
