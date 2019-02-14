#pragma once

#include <bitset>
#include "Awl/BitMap.h"

namespace awl
{
    namespace io
    {
        namespace adapters
        {
            template <std::size_t N>
            class BitSetAdapter
            {
            public:

                typedef bool value_type;
                typedef std::size_t size_type;

                explicit BitSetAdapter(const std::bitset<N> & v) : m_bits(v)
                {
                }

                size_type size() const
                {
                    return m_bits.size();
                }

                //For the compatibility with std::vector<bool>.

                auto at(std::size_t i) const
                {
                    return m_bits[i];
                }

                auto at(std::size_t i)
                {
                    return m_bits[i];
                }

            private:

                std::bitset<N> m_bits;
            };

            template<typename Enum, std::size_t N>
            class BitMapAdapter
            {
            public:

                typedef bool value_type;
                typedef std::size_t size_type;

                explicit BitMapAdapter(const bitmap<Enum, N> & v) : m_bm(v)
                {
                }

                size_type size() const
                {
                    return m_bm.size();
                }

                auto at(std::size_t i) const
                {
                    return m_bm[static_cast<Enum>(i)];
                }

                auto at(std::size_t i)
                {
                    return m_bm[static_cast<Enum>(i)];
                }

            private:

                bitmap<Enum, N> m_bm;
            };
        }
    }
}
