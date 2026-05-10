/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "StreamUtils.h"

#include "Awl/Io/IoException.h"
#include "Awl/Io/SequentialStream.h"
#include "Awl/Io/ReadWrite.h"

#include <algorithm>
#include <memory>
#include <cassert>
namespace awl::io
{
    class SwitchMemoryOutputStream
    {
    public:

        //new uint8_t[size] is not constexpr and allocating 64K on the stack probably is not a good idea.
        SwitchMemoryOutputStream(size_t size) : _size(size), pBuf(new uint8_t[size]), _p(pBuf)
        {
            std::memset(pBuf, 0u, _size);
        }

        ~SwitchMemoryOutputStream()
        {
            delete[] pBuf;
        }

        //To make this look better and get gid of switch operator we would probably define
        //the specialization of Read/Write functions not only for the type
        //but also for the stream.
        constexpr void write(const uint8_t * buffer, size_t count)
        {
            switch (count)
            {
            case 1:
                PlainCopy<uint8_t>(_p, buffer);
                break;
            case 2:
                PlainCopy<uint16_t>(_p, buffer);
                break;
            case 4:
                PlainCopy<uint32_t>(_p, buffer);
                break;
            case 8:
                PlainCopy<uint64_t>(_p, buffer);
                break;
            default:
                //memcpy, memmove, and memset are obsolete!
                //std::copy is constexpr in C++ 20.
                //std::copy(buffer, buffer + count, _p);
                StdCopy(buffer, buffer + count, _p);
                break;
            }

            _p += count;
        }

        size_t GetCapacity() const
        {
            return _size;
        }

        size_t length() const
        {
            return _p - pBuf;
        }

        void Reset()
        {
            _p = pBuf;
        }

        const uint8_t * begin() const { return pBuf; }
        const uint8_t * end() const { return pBuf + _size; }

    private:

        const size_t _size;
        uint8_t * pBuf;
        uint8_t * _p;
    };

    static_assert(sequential_output_stream<SwitchMemoryOutputStream>);

    class VirtualMemoryOutputStream : public awl::io::SequentialOutputStream
    {
    public:

        VirtualMemoryOutputStream(size_t size) : _size(size), pBuf(new uint8_t[size]), _p(pBuf.get())
        {
            std::memset(pBuf.get(), 0u, _size);
        }

        void write(const uint8_t * buffer, size_t count) override
        {
            assert(length() + count <= _size);
            std::memmove(_p, buffer, count);
            //std::copy(buffer, buffer + count, _p);
            _p += count;
        }

        size_t GetCapacity() const
        {
            return _size;
        }

        size_t length() const
        {
            assert(pBuf.get() <= _p);
            return _p - pBuf.get();
        }

        void Reset()
        {
            _p = pBuf.get();
        }

        const uint8_t * begin() const { return pBuf.get(); }
        const uint8_t * end() const { return pBuf.get() + _size; }

    private:

        const size_t _size;
        std::unique_ptr<uint8_t[]> pBuf;
        uint8_t * _p;
    };

    static_assert(sequential_output_stream<VirtualMemoryOutputStream>);

    class VirtualMeasureStream : public awl::io::SequentialOutputStream
    {
    public:

        void write(const uint8_t * buffer, size_t count) override
        {
            static_cast<void>(buffer);
            //compound assignment with volatile-qualified left operand is deprecated
            //_pos += count;
            _pos = _pos + count;
        }

        size_t length() const
        {
            return _pos;
        }

    private:

        //prevent the optimization
        volatile size_t _pos = 0;
    };

    static_assert(sequential_output_stream<VirtualMeasureStream>);

    class InlineMeasureStream
    {
    public:

        void write(const uint8_t * buffer, size_t count)
        {
            static_cast<void>(buffer);
            //compound assignment with volatile-qualified left operand is deprecated
            //_pos += count;
            _pos = _pos + count;
        }

        size_t length() const
        {
            return _pos;
        }

    private:

        //prevent the optimization
        volatile size_t _pos = 0;
    };

    static_assert(sequential_output_stream<InlineMeasureStream>);

    std::unique_ptr<SequentialOutputStream> CreateFakeStream();

    std::unique_ptr<SequentialOutputStream> CreateMeasureStream();
}
