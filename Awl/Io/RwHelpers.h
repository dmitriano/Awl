#pragma once

#include <string>
#include <vector>

namespace awl 
{
    namespace io
    {
        template <class Stream, typename T>
        inline void ReadScalar(Stream & s, T & val)
        {
            const size_t size = sizeof(T);

            s.Read(reinterpret_cast<uint8_t *>(&val), size);
        }

        template <class Stream, typename T>
        inline void WriteScalar(Stream & s, const T & val)
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

            s.Read(reinterpret_cast<uint8_t *>(const_cast<Char *>(val.data())), len * sizeof(Char));

            *(const_cast<Char *>(val.data() + len)) = 0;
        }

        template <class Stream, typename Char>
        void WriteString(Stream & s, const std::basic_string<Char> & val)
        {
            const size_t len = val.length();

            WriteScalar(s, len);

            s.Write(reinterpret_cast<const uint8_t *>(val.data()), len * sizeof(Char));
        }

        template <class Stream, typename T>
        inline void ReadVector(Stream & s, std::vector<T> & items)
        {
            int size = sizeof(T);

            s.Read(reinterpret_cast<uint8_t *>(items.data()), size * items.size());
        }

        template <class Stream, typename T>
        inline void WriteVector(Stream & s, const std::vector<T> & items)
        {
            int size = sizeof(T);

            s.Write(reinterpret_cast<const uint8_t *>(items.data()), size * items.size());
        }

        template <class Stream>
        void ReadVector(Stream & s, std::vector<bool> & x)
        {
            std::vector<bool>::size_type n = x.size();

            for (std::vector<bool>::size_type i = 0; i < n;)
            {
                uint8_t aggr;

                ReadScalar(s, aggr);

                for (unsigned char mask = 1; mask > 0 && i < n; ++i, mask <<= 1)
                {
                    x.at(i) = (aggr & mask) != 0;
                }
            }
        }

        template <class Stream>
        void WriteVector(Stream & s, const std::vector<bool> & x)
        {
            std::vector<bool>::size_type n = x.size();

            for (std::vector<bool>::size_type i = 0; i < n;)
            {
                unsigned char aggr = 0;

                for (unsigned char mask = 1; mask > 0 && i < n; ++i, mask <<= 1)
                {
                    if (x.at(i))
                    {
                        aggr |= mask;
                    }
                }

                WriteScalar(s, aggr);
            }
        }
    }
}
