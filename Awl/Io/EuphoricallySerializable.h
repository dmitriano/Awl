/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/HashingSerializable.h"
#include "Awl/Io/VersionTolerantSerializable.h"
#include "Awl/Io/VectorStream.h"
#include "Awl/Io/MeasureStream.h"
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

            using HashIStream = HashInputStream<Hash, IStream>;
            using HashOStream = HashOutputStream<Hash, OStream>;

            VersionTolerantSerializable<T, V, HashIStream, HashOStream> m_vts;
        };
    }

    template <class T, class V, class IStream = SequentialInputStream, class OStream = SequentialOutputStream,
        class Hash = awl::crypto::Crc64>
    class EuphoricallySerializable :
        private helpers::VtsOwner<T, V, IStream, OStream, Hash>,
        public HashingSerializable<IStream, OStream, Hash>
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

        std::vector<uint8_t> MakeShanshot() const
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
