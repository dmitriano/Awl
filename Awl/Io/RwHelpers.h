#pragma once

#include <string>

namespace awl 
{
    namespace io
    {
        template <class Stream, typename T>
        void ReadScalar(Stream & s, T & val)
        {
            const size_t size = sizeof(T);

            s.Read(reinterpret_cast<uint8_t *>(&val), size);
        }

        template <class Stream, typename T>
        void WriteScalar(Stream & s, const T & val)
        {
            const size_t size = sizeof(T);

            s.Write(reinterpret_cast<const uint8_t *>(&val), size);
        }

        template <class Stream, typename Char>
        void ReadString(Stream & s, std::basic_string<Char> & val)
        {
            size_t len;

            ReadScalar(s, len);

            val.resize(len);

            Read(s, reinterpret_cast<uint8_t *>(const_cast<Char *>(val.data())), len * sizeof(Char));

            *(const_cast<Char *>(val.data() + len)) = 0;
        }

        template <class Stream, typename Char>
        void WriteString(Stream & s, const std::basic_string<Char> & val)
        {
            const size_t len = val.length();

            WriteScalar(s, len);

            Write(s, reinterpret_cast<const uint8_t *>(val.data()), len * sizeof(Char));
        }
    }
}
