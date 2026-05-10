/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Io/IoException.h"
#include "Awl/Io/SequentialStream.h"

#include <vector>
#include <algorithm>
#include <iterator>
#include <cassert>

namespace awl 
{
    namespace io
    {
        class VectorInputStream : public SequentialInputStream
        {
        public:

            VectorInputStream(const std::vector<uint8_t> & v) : _v(v), _i(_v.begin())
            {}

            bool end() override
            {
                return _i == _v.end();
            }

            size_t read(uint8_t * buffer, size_t count) override
            {
                auto diff = _v.end() - _i;

                assert(diff >= 0);

                auto available = static_cast<size_t>(diff);
                
                const size_t read_count = std::min(available, count);
                
                //Can be slow with uint8_t.
                //std::copy(_i, end, stdext::make_checked_array_iterator(buffer, count));

                //This results in an assert if diff == 0
                //const uint8_t * src = &(*_i);

                //Do not call std::memcpy with zero length to avoid GCC Address Sanitizer warnings.
                if (read_count != 0)
                {
                    auto pos = _i - _v.begin();

                    const uint8_t* src = _v.data() + pos;

                    std::memcpy(buffer, src, read_count * sizeof(uint8_t));
                }

                auto end = _i + read_count;

                _i = end;

                return read_count;
            }

        private:

            const std::vector<uint8_t> & _v;

            std::vector<uint8_t>::const_iterator _i;
        };

        class VectorOutputStream : public SequentialOutputStream
        {
        public:

            VectorOutputStream(std::vector<uint8_t> & v) : _v(v)
            {}

            void write(const uint8_t * buffer, size_t count) override
            {
                _v.insert(_v.end(), buffer, buffer + count);
            }

        private:

            std::vector<uint8_t> & _v;
        };
    }
}
