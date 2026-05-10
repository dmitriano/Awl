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

#include <vector>
#include <cstdint>
#include <cassert>
namespace awl::io
{
    namespace helpers
    {
        // A trink to initialize VersionTolerantSerializable before HashingSerializable.
        template <class T, class IStream, class OStream, class Hash, class V>
        class VtsOwner
        {
        protected:

            VtsOwner(T& val) : _vts(val) {}

            VersionTolerantSerializable<T, HashInputStream<Hash, IStream>, HashOutputStream<Hash, OStream>, true, V> _vts;
        };
    }

    template <class T, class IStream = SequentialInputStream, class OStream = SequentialOutputStream,
        class Hash = awl::crypto::Crc64, class V = mp::variant_from_struct<T>>
    class EuphoricallySerializable :
        private helpers::VtsOwner<T, IStream, OStream, Hash, V>,
        public HashingSerializable<IStream, OStream, Hash>,
        public Snapshotable
    {
    private:

        using BaseVts = helpers::VtsOwner<T, IStream, OStream, Hash, V>;
        using BaseHashing = HashingSerializable<IStream, OStream, Hash>;

    public:

        EuphoricallySerializable(T& val, size_t block_size = defaultBlockSize, Hash hash = {}) :
            BaseVts(val),
            BaseHashing(BaseVts::_vts, block_size, hash),
            vector_vts(val),
            measure_vts(val)
        {}

        std::shared_ptr<Snapshot> makeShanshot() const override
        {
            std::vector<uint8_t> v;

            {
                VectorOutputStream out(v);

                this->writeHeader(out);

                std::size_t header_len = v.size();

                const size_t content_len = measureContent();

                const size_t len = header_len + content_len;

                v.reserve(len);

                writeContent(out);

                assert(v.size() == len);
            }

            return BaseHashing::makeShanshotHelper(std::move(v));
        }

    private:

        std::size_t measureContent() const
        {
            MeasureStream out;

            measure_vts.write(out);

            return out.length();
        }

        void writeContent(VectorOutputStream& out) const
        {
            vector_vts.write(out);
        }

        VersionTolerantSerializable<T, VectorInputStream, VectorOutputStream, true, V> vector_vts;
        VersionTolerantSerializable<T, SequentialInputStream, MeasureStream, true, V> measure_vts;
    };
}
