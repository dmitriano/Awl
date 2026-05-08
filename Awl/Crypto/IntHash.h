/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Crypto/Crc64.h"

#include <string>

namespace awl::crypto
{
    class Int64Hash
    {
    public:

        using value_type = uint64_t;

        constexpr Int64Hash(uint64_t seed = 0) : _hash(seed) {}

        template <class I>
        constexpr value_type operator()(I begin, I end) const
        {
            const auto val = _hash(begin, end);
            return from_buffer<value_type>(val);
        }

    private:

        awl::crypto::Crc64 _hash;
    };
}
