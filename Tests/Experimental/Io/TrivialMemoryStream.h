#pragma once

#include "Awl/Io/IoException.h"
#include "Awl/Io/RwHelpers.h"

#include "StreamUtils.h"

namespace awl::io
{
    class TrivialMemoryStream
    {
    public:

        TrivialMemoryStream(size_t size) : m_size(size), pBuf(new uint8_t[size]), m_p(pBuf)
        {
            std::memset(pBuf, 0u, m_size);
        }

        ~TrivialMemoryStream()
        {
            delete pBuf;
        }

        constexpr bool End()
        {
            return GetLength() == m_size;
        }

        /*
        constexpr size_t Read(uint8_t * buffer, size_t count)
        {
            const size_t available_count = m_size - GetLength();
            const size_t read_count = std::min(count, available_count);
            StdCopy(m_p, m_p + read_count, buffer);
            m_p += read_count;
            return read_count;
        }
        */

        constexpr size_t Read(uint8_t * buffer, size_t count)
        {
            StdCopy(m_p, m_p + count, buffer);
            m_p += count;
            return count;
        }

        template <class T>
        constexpr std::enable_if_t<std::is_arithmetic_v<T>, void> ReadArithmetic(T & val)
        {
            val = *(reinterpret_cast<T *>(m_p));
            m_p += sizeof(val);
        }

        constexpr void Write(const uint8_t * buffer, size_t count)
        {
            //std::memmove(m_p, buffer, count);
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

        constexpr size_t GetCapacity() const
        {
            return m_size;
        }

        constexpr size_t GetLength() const
        {
            return static_cast<size_t>(m_p - pBuf);
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
    inline std::enable_if_t<std::is_arithmetic_v<T> && !std::is_same_v<T, bool>, void> Write(TrivialMemoryStream & s, T val)
    {
        s.WriteArithmetic(val);
    }

    template <>
    inline void Write(TrivialMemoryStream & s, bool b)
    {
        uint8_t val = b ? 1 : 0;
        s.WriteArithmetic(val);
    }

    template <typename T>
    inline typename std::enable_if<std::is_arithmetic<T>::value && !std::is_same<T, bool>::value, void>::type Read(TrivialMemoryStream & s, T & val)
    {
        s.ReadArithmetic(val);
    }

    template <>
    inline void Read(TrivialMemoryStream & s, bool & b)
    {
        uint8_t val;
        s.ReadArithmetic(val);
        b = val != 0;
    }
}
