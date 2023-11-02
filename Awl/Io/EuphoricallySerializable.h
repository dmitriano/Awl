/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/HashingSerializable.h"
#include "Awl/Io/VersionTolerantSerializable.h"

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
            BaseHashing(BaseVts::m_vts, block_size, hash)
        {}
    };
}
