/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>

namespace awl::crypto
{
    //Can be calculated in compile time with Int64Hash, for example.
    template <class Hash>
    class FixedHash
    {
    public:

        using value_type = typename Hash::value_type;

        static constexpr size_t size()
        {
            return Hash::size();
        }

        explicit constexpr FixedHash(Hash h = {}) : m_hash(h)
        {}

        template <typename C>
        value_type operator()(const std::basic_string<C>& str) const
        {
            return m_hash(str.begin(), str.end());
        }

        template <typename C>
        value_type operator()(const std::basic_string_view<C>& str) const
        {
            return m_hash(str.begin(), str.end());
        }

        template <typename C, size_t N>
        constexpr value_type operator()(const C(&s)[N]) const
        {
            static_assert(N >= 1, "The parameter is not a string literal.");

            constexpr size_t length = N - 1;

            return m_hash(s, s + length);
        }

        template <class I>
        value_type operator()(I begin, I end) const
        {
            return m_hash(begin, end);
        }

    private:

        Hash m_hash;
    };
}
