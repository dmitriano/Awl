#pragma once

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

            bool End() const override
            {
                return m_i == m_v.end();
            }

            void Read(uint8_t * buffer, size_t count) override
            {
                auto diff = m_v.end() - m_i;

                assert(diff >= 0);

                if (static_cast<size_t>(diff) < count)
                {
                    throw EndOfFileException();
                }

                auto end = m_i + count;

#if defined(_MSC_VER)
                auto dest_iter = stdext::make_checked_array_iterator(buffer, count);
                
#else
                auto dest_iter = buffer;
#endif

                std::copy(m_i, end, dest_iter);

                m_i = end;
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
