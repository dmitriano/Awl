/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/BitMap.h"

#include <bitset>

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

                explicit BitSetAdapter(BitSet & v) : _bits(v)
                {}

                size_type size() const
                {
                    return _bits.size();
                }

                auto at(std::size_t i) const
                {
                    return _bits[i];
                }

                auto at(std::size_t i)
                {
                    return _bits[i];
                }

            private:

                BitSet & _bits;
            };

            template<class BitMap>
            class BitMapAdapter
            {
            public:

                using value_type = bool;
                using size_type = typename BitMap::size_type;

                explicit BitMapAdapter(BitMap & v) : _bm(v)
                {}

                size_type size() const
                {
                    return _bm.size();
                }

                auto at(size_type i) const
                {
                    return _bm[static_cast<typename BitMap::enum_type>(i)];
                }

                auto at(size_type i)
                {
                    return _bm[static_cast<typename BitMap::enum_type>(i)];
                }

            private:

                BitMap & _bm;
            };
        }
    }
}
