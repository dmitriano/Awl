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
        constexpr size_t defaultBlockSize = 1024 * 64;
        
        template <class Hash, class UnderlyingStream = SequentialInputStream>
        class HashInputStream : public SequentialInputStream
        {
        public:

            HashInputStream(UnderlyingStream& in, size_t block_size = defaultBlockSize, Hash hash = {}) : m_hash(hash), m_in(in), blockSize(block_size), m_i(m_block.end())
            {
                assert(blockSize > Hash::size());
            }

            HashInputStream(UnderlyingStream& in, Hash hash) : HashInputStream(in, defaultBlockSize, hash)
            {
            }

            bool End() override
            {
                return InternalEnd();
            }

            size_t Read(uint8_t * buffer, size_t count) override;

        private:

            bool InternalEnd()
            {
                PeekBuf();

                return m_i == m_block.end();
            }
            
            void PeekBuf()
            {
                if (m_i == m_block.end())
                {
                    m_block.resize(blockSize);

                    const size_t actually_read = m_in.Read(m_block.data(), blockSize);

                    if (actually_read == 0)
                    {
                        //Nothing to read, we are at the end of the file.
                        m_block.clear();
                    }
                    else
                    {
                        assert(actually_read <= blockSize);

                        if (actually_read < Hash::size())
                        {
                            throw CorruptionException();
                        }

                        m_block.resize(actually_read);

                        typename Hash::value_type read_val;

                        auto hash_begin = m_block.begin() + actually_read - Hash::size();

                        for (size_t i = 0; i != read_val.size(); ++i)
                        {
                            read_val[i] = *(hash_begin + i);
                        }

                        auto calculated_val = m_hash(m_block.begin(), hash_begin);

                        if (calculated_val != read_val)
                        {
                            throw CorruptionException();
                        }

                        m_block.resize(actually_read - Hash::size());
                    }

                    m_i = m_block.begin();
                }
            }

            void FlushBuf(uint8_t * buffer, size_t & flushed_count, size_t count)
            {
                assert(m_i <= m_block.end());
                
                if (m_i != m_block.end())
                {
                    const size_t available_count = static_cast<size_t>(m_block.end() - m_i);

                    const size_t remaining_count = count - flushed_count;

                    const size_t write_count = std::min(available_count, remaining_count);

                    std::memcpy(buffer + flushed_count, static_cast<const uint8_t *>(&(*m_i)), write_count);

                    m_i += write_count;

                    flushed_count += write_count;
                }
            }

            const Hash m_hash;
            
            UnderlyingStream& m_in;
            
            const size_t blockSize;

            std::vector<uint8_t> m_block;

            std::vector<uint8_t>::iterator m_i;
        };

        template <class Hash, class UnderlyingStream>
        size_t HashInputStream<Hash, UnderlyingStream>::Read(uint8_t * buffer, size_t count)
        {
            size_t flushed_count = 0;

            while (true)
            {
                FlushBuf(buffer, flushed_count, count);

                assert(flushed_count <= count);

                if (flushed_count == count)
                {
                    break;
                }

                if (InternalEnd())
                {
                    break;
                }
            }

            assert(flushed_count <= count);

            return flushed_count;
        }

        template <class Hash, class UnderlyingStream = SequentialOutputStream>
        class HashOutputStream : public SequentialOutputStream
        {
        public:

            HashOutputStream(UnderlyingStream& out, size_t block_size = defaultBlockSize, Hash hash = {}) :
                m_hash(hash), m_out(out), blockSize(block_size)
            {
                assert(blockSize > Hash::size());

                m_v.reserve(blockSize);
            }

            HashOutputStream(UnderlyingStream& out, Hash hash) : HashOutputStream(out, defaultBlockSize, hash)
            {
            }

            ~HashOutputStream()
            {
                Flush();
            }

            void Write(const uint8_t * buffer, size_t count) override;

        private:

            void Flush()
            {
                if (!m_v.empty())
                {
                    auto val = m_hash(m_v.begin(), m_v.end());

                    m_v.insert(m_v.end(), val.begin(), val.end());

                    m_out.Write(m_v.data(), m_v.size());

                    m_v.clear();
                }
            }

            Hash m_hash;

            UnderlyingStream& m_out;

            const size_t blockSize;

            std::vector<uint8_t> m_v;
        };

        template <class Hash, class UnderlyingStream>
        void HashOutputStream<Hash, UnderlyingStream>::Write(const uint8_t * buffer, size_t count)
        {
            assert(m_v.size() < blockSize - Hash::size());

            size_t written_cont = 0;

            do
            {
                const size_t remaining_cont = count - written_cont;
                
                const size_t tail_count = blockSize - Hash::size() - m_v.size();

                const size_t insert_count = std::min(tail_count, remaining_cont);

                m_v.insert(m_v.end(), buffer + written_cont, buffer + written_cont + insert_count);

                assert(m_v.size() <= blockSize - Hash::size());

                if (m_v.size() == blockSize - Hash::size())
                {
                    Flush();
                }

                written_cont += insert_count;

                assert(written_cont <= count);
            }
            while (written_cont != count);
        }
    }
}
