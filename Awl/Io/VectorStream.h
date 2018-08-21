#pragma once

#include "Awl/Io/SequentialStream.h"

#include <vector>
#include <algorithm>
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

            bool End() const
            {
                return m_i == m_v.end();
            }

            size_t Read(uint8_t * buffer, size_t count) override
            {
                auto diff = m_v.end() - m_i;

                assert(diff >= 0);

                auto actual_count = std::min(count, static_cast<size_t>(diff));

                auto end = m_i + actual_count;

                std::copy(m_i, end, buffer);
                
                m_i = end;

                return actual_count;
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

            size_t Write(const uint8_t * buffer, size_t count) override
            {
                m_v.insert(m_v.end(), buffer, buffer + count);

                return count;
            }

        private:

            std::vector<uint8_t> & m_v;
        };
    }
}
