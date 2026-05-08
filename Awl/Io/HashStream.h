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
        constexpr size_t defaultBlockSize = 1024 * 64;
        
        template <class Hash, class UnderlyingStream = SequentialInputStream>
        class HashInputStream : public SequentialInputStream
        {
        public:

            HashInputStream(UnderlyingStream& in, size_t block_size = defaultBlockSize, Hash hash = {}) : _hash(hash), _in(in), blockSize(block_size), _i(_block.end())
            {
                assert(blockSize > Hash::size());
            }

            HashInputStream(UnderlyingStream& in, Hash hash) : HashInputStream(in, defaultBlockSize, hash)
            {}

            bool end() override
            {
                return internalEnd();
            }

            size_t read(uint8_t * buffer, size_t count) override;

        private:

            bool internalEnd()
            {
                peekBuf();

                return _i == _block.end();
            }
            
            void peekBuf()
            {
                if (_i == _block.end())
                {
                    _block.resize(blockSize);

                    const size_t actually_read = _in.read(_block.data(), blockSize);

                    if (actually_read == 0)
                    {
                        //Nothing to read, we are at the end of the file.
                        _block.clear();
                    }
                    else
                    {
                        assert(actually_read <= blockSize);

                        if (actually_read < Hash::size())
                        {
                            throw CorruptionException();
                        }

                        _block.resize(actually_read);

                        typename Hash::value_type read_val;

                        auto hash_begin = _block.begin() + actually_read - Hash::size();

                        for (size_t i = 0; i != read_val.size(); ++i)
                        {
                            read_val[i] = *(hash_begin + i);
                        }

                        auto calculated_val = _hash(_block.begin(), hash_begin);

                        if (calculated_val != read_val)
                        {
                            throw CorruptionException();
                        }

                        _block.resize(actually_read - Hash::size());
                    }

                    _i = _block.begin();
                }
            }

            void flushBuf(uint8_t * buffer, size_t & flushed_count, size_t count)
            {
                assert(_i <= _block.end());
                
                if (_i != _block.end())
                {
                    const size_t available_count = static_cast<size_t>(_block.end() - _i);

                    const size_t remaining_count = count - flushed_count;

                    const size_t write_count = std::min(available_count, remaining_count);

                    std::memcpy(buffer + flushed_count, static_cast<const uint8_t *>(&(*_i)), write_count);

                    _i += write_count;

                    flushed_count += write_count;
                }
            }

            const Hash _hash;
            
            UnderlyingStream& _in;
            
            const size_t blockSize;

            std::vector<uint8_t> _block;

            std::vector<uint8_t>::iterator _i;
        };

        template <class Hash, class UnderlyingStream>
        size_t HashInputStream<Hash, UnderlyingStream>::read(uint8_t * buffer, size_t count)
        {
            size_t flushed_count = 0;

            while (true)
            {
                flushBuf(buffer, flushed_count, count);

                assert(flushed_count <= count);

                if (flushed_count == count)
                {
                    break;
                }

                if (internalEnd())
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
                _hash(hash), _out(out), blockSize(block_size)
            {
                assert(blockSize > Hash::size());

                _v.reserve(blockSize);
            }

            HashOutputStream(UnderlyingStream& out, Hash hash) : HashOutputStream(out, defaultBlockSize, hash)
            {}

            ~HashOutputStream()
            {
                flush();
            }

            void write(const uint8_t * buffer, size_t count) override;

        private:

            void flush()
            {
                if (!_v.empty())
                {
                    auto val = _hash(_v.begin(), _v.end());

                    _v.insert(_v.end(), val.begin(), val.end());

                    _out.write(_v.data(), _v.size());

                    _v.clear();
                }
            }

            Hash _hash;

            UnderlyingStream& _out;

            const size_t blockSize;

            std::vector<uint8_t> _v;
        };

        template <class Hash, class UnderlyingStream>
        void HashOutputStream<Hash, UnderlyingStream>::write(const uint8_t * buffer, size_t count)
        {
            assert(_v.size() < blockSize - Hash::size());

            size_t written_cont = 0;

            do
            {
                const size_t remaining_cont = count - written_cont;
                
                const size_t tail_count = blockSize - Hash::size() - _v.size();

                const size_t insert_count = std::min(tail_count, remaining_cont);

                _v.insert(_v.end(), buffer + written_cont, buffer + written_cont + insert_count);

                assert(_v.size() <= blockSize - Hash::size());

                if (_v.size() == blockSize - Hash::size())
                {
                    flush();
                }

                written_cont += insert_count;

                assert(written_cont <= count);
            }
            while (written_cont != count);
        }
    }
}
