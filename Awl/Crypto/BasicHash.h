/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Int2Array.h"

#include <stdint.h>
#include <array>
#include <type_traits>
#include <algorithm>

namespace awl
{
    namespace crypto
    {
        //The standard library does not impose specific algorithms for calculating hashes, so we won't get portable hash codes from the standard library,
        //even for the same system, if we use a different compiler. So we need our own.

        template <size_t N> using HashValue = std::array<uint8_t, N>;

        template <size_t N>
        class BasicHash
        {
        private:

            static constexpr size_t value_size = N;

        public:

            static constexpr size_t size()
            {
                return value_size;
            }

            using value_type = HashValue<value_size>;
        };

        class FakeHash : public BasicHash<0u>
        {
        public:

            template <class InputIt>
            constexpr value_type operator()(InputIt begin, InputIt end) const
            {
                static_cast<void>(begin);
                static_cast<void>(end);
                return {};
            }
        };

        template <class Hash>
        class ReverseHash : public BasicHash<Hash::size()>
        {
        private:

            using Base = BasicHash<Hash::size()>;

        public:

            using value_type = typename Base::value_type;

            explicit constexpr ReverseHash(Hash hash) : m_hash(std::move(hash)) {}

            template <class InputIt>
            constexpr value_type operator()(InputIt begin, InputIt end) const
            {
                value_type val = m_hash(begin, end);

                std::reverse(val.begin(), val.end());

                return val;
            }

        private:

            Hash m_hash;
        };
    }
}
