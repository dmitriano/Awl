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
        SwitchMemoryOutputStream(size_t size) : m_size(size), pBuf(new uint8_t[size]), m_p(pBuf)
        {
            std::memset(pBuf, 0u, m_size);
        }

        ~SwitchMemoryOutputStream()
        {
            delete pBuf;
        }

        //To make this look better and get gid of switch operator we would probably define
        //the specialization of Read/Write functions not only for the type
        //but also for the stream.
        constexpr void Write(const uint8_t * buffer, size_t count)
        {
            switch (count)
            {
            case 1:
                PlainCopy<uint8_t>(m_p, buffer);
                break;
            case 2:
                PlainCopy<uint16_t>(m_p, buffer);
                break;
            case 4:
                PlainCopy<uint32_t>(m_p, buffer);
                break;
            case 8:
                PlainCopy<uint64_t>(m_p, buffer);
                break;
            default:
                //memcpy, memmove, and memset are obsolete!
                //std::copy is constexpr in C++ 20.
                //std::copy(buffer, buffer + count, m_p);
                StdCopy(buffer, buffer + count, m_p);
                break;
            }

            m_p += count;
        }

        size_t GetCapacity() const
        {
            return m_size;
        }

        size_t GetLength() const
        {
            return m_p - pBuf;
        }

        void Reset()
        {
            m_p = pBuf;
        }

        const uint8_t * begin() const { return pBuf; }
        const uint8_t * end() const { return pBuf + m_size; }

    private:

        const size_t m_size;
        uint8_t * pBuf;
        uint8_t * m_p;
    };

    class VirtualMemoryOutputStream : public awl::io::SequentialOutputStream
    {
    public:

        VirtualMemoryOutputStream(size_t size) : m_size(size), pBuf(new uint8_t[size]), m_p(pBuf.get())
        {
            std::memset(pBuf.get(), 0u, m_size);
        }

        void Write(const uint8_t * buffer, size_t count) override
        {
            assert(GetLength() + count <= m_size);
            std::memmove(m_p, buffer, count);
            //std::copy(buffer, buffer + count, m_p);
            m_p += count;
        }

        size_t GetCapacity() const
        {
            return m_size;
        }

        size_t GetLength() const
        {
            assert(pBuf.get() <= m_p);
            return m_p - pBuf.get();
        }

        void Reset()
        {
            m_p = pBuf.get();
        }

        const uint8_t * begin() const { return pBuf.get(); }
        const uint8_t * end() const { return pBuf.get() + m_size; }

    private:

        const size_t m_size;
        std::unique_ptr<uint8_t[]> pBuf;
        uint8_t * m_p;
    };

    class VirtualMeasureStream : public awl::io::SequentialOutputStream
    {
    public:

        void Write(const uint8_t * buffer, size_t count) override
        {
            static_cast<void>(buffer);
            //compound assignment with ‘volatile’-qualified left operand is deprecated
            //m_pos += count;
            m_pos = m_pos + count;
        }

        size_t GetLength() const
        {
            return m_pos;
        }

    private:

        //prevent the optimization
        volatile size_t m_pos = 0;
    };

    class InlineMeasureStream
    {
    public:

        void Write(const uint8_t * buffer, size_t count)
        {
            static_cast<void>(buffer);
            //compound assignment with ‘volatile’-qualified left operand is deprecated
            //m_pos += count;
            m_pos = m_pos + count;
        }

        size_t GetLength() const
        {
            return m_pos;
        }

    private:

        //prevent the optimization
        volatile size_t m_pos = 0;
    };

    std::unique_ptr<SequentialOutputStream> CreateFakeStream();

    std::unique_ptr<SequentialOutputStream> CreateMeasureStream();
}
