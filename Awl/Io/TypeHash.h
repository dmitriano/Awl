#pragma once

#include "Awl/Crypto/Crc64.h"
#include "Awl/Io/TypeName.h"

namespace awl::io
{
    namespace helpers
    {
        class Int64Hash
        {
        public:

            using value_type = uint64_t;

            constexpr Int64Hash(uint64_t seed = 0) : m_hash(seed) {}

            value_type operator()(std::string::const_iterator begin, std::string::const_iterator end) const
            {
                return (*this)(&(*begin), &(*begin) + (end - begin));
            }

            constexpr value_type operator()(const char * begin, const char * end) const
            {
                return awl::crypto::from_array<value_type>(m_hash(begin, end));
            }

        private:

            awl::crypto::Crc64 m_hash;
        };
    }

    using TypeId = helpers::Int64Hash::value_type;

    template <size_t N>
    constexpr TypeId calc_type_hash(const FixedString<N> & name)
    {
        helpers::Int64Hash hash;
        return hash(name.begin(), name.end());
    }

    template <class T>
    constexpr TypeId make_type_hash()
    {
        auto name = make_type_name<T>();
        return calc_type_hash(name);
    }

    static_assert(make_type_hash<uint8_t>() != make_type_hash<int16_t>());
}
