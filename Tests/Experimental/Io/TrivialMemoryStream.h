/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "StreamUtils.h"

#include "Awl/Io/IoException.h"
#include "Awl/Io/ReadWrite.h"

#include <cassert>
#include <cstddef>
#include <cstring>

namespace awl::io
{
    class TrivialMemoryStream
    {
    public:

        TrivialMemoryStream(size_t size) : _size(size), pBuf(new std::byte[size]), _p(pBuf)
        {
            std::memset(pBuf, 0u, _size);
        }

        ~TrivialMemoryStream()
        {
            delete[] pBuf;
        }

        constexpr bool end()
        {
            return length() == _size;
        }

        constexpr size_t read(std::byte * buffer, size_t count)
        {
            const size_t available_count = _size - length();
            const size_t read_count = std::min(count, available_count);
            StdCopy(_p, _p + read_count, buffer);
            _p += read_count;
            return read_count;
        }

        constexpr void write(const std::byte * buffer, size_t count)
        {
            assert(length() + count <= _size);
            //std::memmove(_p, buffer, count);
            StdCopy(buffer, buffer + count, _p);
            _p += count;
        }

        constexpr size_t GetCapacity() const
        {
            return _size;
        }

        constexpr size_t length() const
        {
            assert(pBuf <= _p);
            return static_cast<size_t>(_p - pBuf);
        }

        void Reset()
        {
            _p = pBuf;
        }

        const std::byte * begin() const { return pBuf; }
        const std::byte * end() const { return pBuf + _size; }

    private:

        template <typename T>
            requires (std::is_arithmetic_v<T> && !std::is_same_v<T, bool>)
        friend void write(TrivialMemoryStream& s, T val)
        {
            assert(s.length() + sizeof(val) <= s._size);
            std::memcpy(s._p, awl::byte_cast(&val), sizeof(val));
            s._p += sizeof(val);
        }

        template <typename T>
            requires (std::is_arithmetic_v<T> && !std::is_same_v<T, bool>)
        friend void read(TrivialMemoryStream& s, T& val)
        {
            assert(s.length() + sizeof(val) <= s._size);
            std::memcpy(awl::byte_cast(&val), s._p, sizeof(val));
            s._p += sizeof(val);
        }

        const size_t _size;
        std::byte * pBuf;
        std::byte * _p;
    };
}
