#pragma once

#include "Awl/Io/IoException.h"
#include "Awl/Io/RwHelpers.h"

namespace awl::io
{
    constexpr void StdCopy(const uint8_t * begin, const uint8_t * end, uint8_t * out)
    {
        const uint8_t * p = begin;
        while (p != end)
        {
            *out++ = *p++;
        }
    }

    class MemoryOutputStream
    {
    public:

        MemoryOutputStream(size_t size) : m_size(size), pBuf(new uint8_t[size]), m_p(pBuf)
        {
            std::memset(pBuf, 0u, m_size);
        }

        ~MemoryOutputStream()
        {
            delete pBuf;
        }

        constexpr void Write(const uint8_t * buffer, size_t count)
        {
            //std::memmove(m_p, buffer, count);
            StdCopy(buffer, buffer + count, m_p);
            m_p += count;
        }

        template <class T>
        std::enable_if_t<std::is_arithmetic_v<T>, void> WriteArithmetic(const T val)
        {
            uint8_t * const new_p = m_p + sizeof(val);

            if (static_cast<size_t>(new_p - pBuf) > m_size)
            {
                throw GeneralException(_T("overflow"));
            }

            *(reinterpret_cast<T *>(m_p)) = val;
            m_p = new_p;
        }

        /*
        template <class T>
        constexpr std::enable_if_t<std::is_arithmetic_v<T>, void> WriteArithmetic(const T val)
        {
            *(reinterpret_cast<T *>(m_p)) = val;
            m_p += sizeof(val);
        }
        */

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

        //how to declare Write specializaion as a friend?
        //template <class Stream, class T> friend void Write(Stream & s, T val);

        const size_t m_size;
        uint8_t * pBuf;
        uint8_t * m_p;
    };

    template <typename T>
    inline std::enable_if_t<std::is_arithmetic_v<T> && !std::is_same_v<T, bool>, void> Write(MemoryOutputStream & s, T val)
    {
        s.WriteArithmetic(val);
    }

    template <>
    inline void Write(MemoryOutputStream & s, bool b)
    {
        uint8_t val = b ? 1 : 0;

        s.WriteArithmetic(val);
    }

    template <class T>
    constexpr inline void PlainCopy(uint8_t * p_dest, const uint8_t * p_src)
    {
        T * dest = reinterpret_cast<T *>(p_dest);
        const T * src = reinterpret_cast<const T *>(p_src);
        *dest = *src;
    }

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
        std::unique_ptr<uint8_t> pBuf;
        uint8_t * m_p;
    };
}
