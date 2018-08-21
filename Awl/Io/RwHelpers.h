#pragma once

#include <string>

namespace awl 
{
    namespace io
    {
        /*
        class IoException
        {
        };

        class ReadException : public IoException
        {
        };
        
        class WriteException : public IoException
        {
        };
        */

        template <class Stream, typename T>
        auto ReadScalar(Stream & s, T & val)
        {
            const size_t size = sizeof(T);

            return s.Read(reinterpret_cast<uint8_t *>(&val), size);

            /*
            if (actually_read != size)
            {

            }
            */
        }

        template <class Stream, typename T>
        auto WriteScalar(Stream & s, const T & val)
        {
            const size_t size = sizeof(T);

            return s.Write(reinterpret_cast<const uint8_t *>(&val), size);
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
        auto WriteString(Stream & s, const std::basic_string<Char> & val)
        {
            const size_t len = val.length();

            WriteScalar(s, len);

            Write(s, reinterpret_cast<const uint8_t *>(val.data()), len * sizeof(Char));
        }
    }
}
