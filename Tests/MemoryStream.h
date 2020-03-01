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
            //std::memset(pBuf, 0u, m_size);
        }

        ~MemoryOutputStream()
        {
            delete pBuf;
        }

        constexpr void Write(const uint8_t * buffer, size_t count)
        {
            StdCopy(buffer, buffer + count, m_p);
            m_p += count;
        }

        /*
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
        */

        template <class T>
        constexpr std::enable_if_t<std::is_arithmetic_v<T>, void> WriteArithmetic(const T val)
        {
            *(reinterpret_cast<T *>(m_p)) = val;
            m_p += sizeof(val);
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

        //how to declare Write specializaion as a friend?
        //template <class Stream, class T> friend void Write(Stream & s, T val);

        const size_t m_size;
        uint8_t * pBuf;
        uint8_t * m_p;
    };

    template <typename T>
    constexpr std::enable_if_t<std::is_arithmetic_v<T> && !std::is_same_v<T, bool>, void> Write(MemoryOutputStream & s, T val)
    {
        s.WriteArithmetic(val);
    }

    template <>
    constexpr void Write(MemoryOutputStream & s, bool b)
    {
        uint8_t val = b ? 1 : 0;

        s.WriteArithmetic(val);
    }
}
