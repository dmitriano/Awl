#pragma once

#include "Awl/Io/IoException.h"
#include "Awl/Io/SequentialStream.h"

#include <vector>
#include <algorithm>
#include <iterator>
#include <assert.h>

namespace awl 
{
    namespace io
    {
        class VectorInputStream : public SequentialInputStream
        {
        public:

            VectorInputStream(const std::vector<uint8_t> & v) : m_v(v), m_i(m_v.begin())
            {
            }

            bool End() override
            {
                return m_i == m_v.end();
            }

            size_t Read(uint8_t * buffer, size_t count) override
            {
                auto diff = m_v.end() - m_i;

                assert(diff >= 0);

                auto available = static_cast<size_t>(diff);
                
                const size_t read_count = std::min(available, count);
                
                //Can be slow with uint8_t.
                //std::copy(m_i, end, stdext::make_checked_array_iterator(buffer, count));

                //This results in an assert if diff == 0
                //const uint8_t * src = &(*m_i);

                auto pos = m_i - m_v.begin();

                const uint8_t * src = m_v.data() + pos;

                std::memcpy(buffer, src, read_count * sizeof(uint8_t));

                auto end = m_i + read_count;

                m_i = end;

                return read_count;
            }

        private:

            const std::vector<uint8_t> & m_v;

            std::vector<uint8_t>::const_iterator m_i;
        };

        class VectorOutputStream : public SequentialOutputStream
        {
        public:

            VectorOutputStream(std::vector<uint8_t> & v) : m_v(v)
            {
            }

            void Write(const uint8_t * buffer, size_t count) override
            {
                m_v.insert(m_v.end(), buffer, buffer + count);
            }

        private:

            std::vector<uint8_t> & m_v;
        };
    }
}
