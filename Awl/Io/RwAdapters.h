#pragma once

#include <bitset>
#include "Awl/BitMap.h"

namespace awl
{
    namespace io
    {
        namespace adapters
        {
            //For the compatibility with std::vector<bool> there should be value_type, size_type, size(), at().
            //Adapters template arguments can be both const and non-const types.

            template <class BitSet>
            class BitSetAdapter
            {
            public:

                using value_type = bool;
                using size_type = std::size_t;

                explicit BitSetAdapter(BitSet & v) : m_bits(v)
                {
                }

                size_type size() const
                {
                    return m_bits.size();
                }

                auto at(std::size_t i) const
                {
                    return m_bits[i];
                }

                auto at(std::size_t i)
                {
                    return m_bits[i];
                }

            private:

                BitSet & m_bits;
            };

            template<class BitMap>
            class BitMapAdapter
            {
            public:

                using value_type = bool;
                using size_type = typename BitMap::size_type;

                explicit BitMapAdapter(BitMap & v) : m_bm(v)
                {
                }

                size_type size() const
                {
                    return m_bm.size();
                }

                auto at(size_type i) const
                {
                    return m_bm[static_cast<typename BitMap::enum_type>(i)];
                }

                auto at(size_type i)
                {
                    return m_bm[static_cast<typename BitMap::enum_type>(i)];
                }

            private:

                BitMap & m_bm;
            };
        }
    }
}
