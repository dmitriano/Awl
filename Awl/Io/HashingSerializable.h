/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/Serializable.h"
#include "Awl/Io/HashStream.h"
#include "Awl/Io/HashingSnapshot.h"
#include "Awl/Io/Rw/VectorReadWrite.h"
#include "Awl/Io/VectorStream.h"
#include "Awl/Crypto/Crc64.h"

#include <vector>
#include <memory>
#include <cstdint>

namespace awl::io
{
    template <class IStream = SequentialInputStream, class OStream = SequentialOutputStream, class Hash = awl::crypto::Crc64>
    class HashingSerializable :
        public Serializable<IStream, OStream>,
        public Snapshotable<OStream>
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
            HashIStream in{ s, m_blockSize, m_hash };

            if (ReadHeader(in))
            {
                m_val.Read(in);
            }
        }

        void Write(OStream& s) const override
        {
            HashOStream out{ s, m_blockSize, m_hash };

            WriteHeader(out);

            m_val.Write(out);
        }

        virtual std::shared_ptr<Snapshot<OStream>> MakeShanshot() const override
        {
            std::vector<uint8_t> v;

            {
                VectorOutputStream out(v);

                WriteHeader(out);

                std::size_t header_len = v.size();

                const size_t len = MeasureContent();

                v.reserve(header_len + len);

                WriteContent(out);

                assert(v.size() == len);
            }

            return std::make_shared<HashingSnapshot<OStream, Hash>>(std::move(v), m_blockSize, m_hash);
        }

    protected:

        virtual bool ReadHeader(awl::io::SequentialInputStream&) { return true; }

        virtual void WriteHeader(awl::io::SequentialOutputStream&) const {}

        virtual std::size_t MeasureContent() const = 0;
            
        virtual void WriteContent(VectorOutputStream& out) const = 0;

    private:

        Value& m_val;
        size_t m_blockSize;
        Hash m_hash;
    };
}
