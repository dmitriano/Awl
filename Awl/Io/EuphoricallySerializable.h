/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/HashingSerializable.h"
#include "Awl/Io/VersionTolerantSerializable.h"
#include "Awl/Io/VectorStream.h"
#include "Awl/Io/MeasureStream.h"
#include "Awl/Io/Snapshotable.h"
#include "Awl/Io/Rw/VectorReadWrite.h"

#include <vector>
#include <cstdint>
#include <cassert>

namespace awl::io
{
    namespace helpers
    {
        // A trink to initialize VersionTolerantSerializable before HashingSerializable.
        template <class T, class V, class IStream, class OStream, class Hash>
        class VtsOwner
        {
        protected:

            VtsOwner(T& val) : m_vts(val) {}

            VersionTolerantSerializable<T, V, HashInputStream<Hash, IStream>, HashOutputStream<Hash, OStream>> m_vts;
        };
    }

    template <class T, class V, class IStream = SequentialInputStream, class OStream = SequentialOutputStream,
        class Hash = awl::crypto::Crc64>
    class EuphoricallySerializable :
        private helpers::VtsOwner<T, V, IStream, OStream, Hash>,
        public HashingSerializable<IStream, OStream, Hash>,
        public Snapshotable<OStream>
    {
    private:

        using BaseVts = helpers::VtsOwner<T, V, IStream, OStream, Hash>;
        using BaseHashing = HashingSerializable<IStream, OStream, Hash>;

    public:

        EuphoricallySerializable(T& val, size_t block_size = defaultBlockSize, Hash hash = {}) :
            BaseVts(val),
            BaseHashing(BaseVts::m_vts, block_size, hash),
            vector_vts(val),
            measure_vts(val)
        {}

        std::vector<uint8_t> MakeShanshot() const override
        {
            const size_t len = MeasureValue();

            std::vector<uint8_t> v;

            v.reserve(len);

            {
                VectorOutputStream out(v);

                vector_vts.Write(out);
            }

            assert(v.size() == len);

            return v;
        }

        void WriteSnapshot(OStream& out, const std::vector<uint8_t>& v) override
        {
            typename BaseHashing::HashOStream hashing_out = BaseHashing::MakeHashingOutputStream(out);

            BaseHashing::WriteHeader(hashing_out);

            // Write vector without leading 8 bytes containing its size.
            hashing_out.Write(v.data(), v.size());
        }

    private:

        size_t MeasureValue() const
        {
            MeasureStream out;

            measure_vts.Write(out);

            return out.GetLength();
        }

        VersionTolerantSerializable<T, V, VectorInputStream, VectorOutputStream> vector_vts;
        VersionTolerantSerializable<T, V, SequentialInputStream, MeasureStream> measure_vts;
    };
}
