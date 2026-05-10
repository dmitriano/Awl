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
            _val(val),
            _blockSize(block_size),
            _hash(hash)
        {}

        void read(IStream& s) override
        {
            HashIStream in{ s, _blockSize, _hash };

            if (readHeader(in))
            {
                _val.read(in);
            }
        }

        void write(OStream& s) const override
        {
            HashOStream out{ s, _blockSize, _hash };

            writeHeader(out);

            _val.write(out);
        }

    protected:

        virtual bool readHeader(awl::io::SequentialInputStream&) { return true; }

        virtual void writeHeader(awl::io::SequentialOutputStream&) const {}

        std::shared_ptr<Snapshot> makeShanshotHelper(std::vector<uint8_t> v) const
        {
            return std::make_shared<HashingSnapshot<Hash>>(std::move(v), _blockSize, _hash);
        }

    private:

        Value& _val;
        size_t _blockSize;
        Hash _hash;
    };
}
