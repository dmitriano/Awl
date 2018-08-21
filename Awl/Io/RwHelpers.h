#pragma once

#include <string>
#include <vector>
#include <chrono>

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

        //Initially on Windows the type was long long, but Android build fails with it, so I changed it to int64_t, what is the difference between them?
        static_assert(std::is_same <std::chrono::nanoseconds::rep, int64_t>::value, "nanoseconds::rep is not int64_t");

        template <class Stream, class Clock, class Duration>
        inline void ReadScalar(Stream & s, std::chrono::time_point<Clock, Duration> & val)
        {
            using namespace std::chrono;

            int64_t ns_count;

            ReadScalar(s, ns_count);

            val = std::chrono::time_point<Clock, Duration>(duration_cast<Duration>(nanoseconds(ns_count)));
        }

        template <class Stream, class Clock, class Duration>
        inline void WriteScalar(Stream & s, const std::chrono::time_point<Clock, Duration> & val)
        {
            using namespace std::chrono;

            auto ns = duration_cast<nanoseconds>(val.time_since_epoch());

            int64_t ns_count = ns.count();

            WriteScalar(s, ns_count);
        }

        template <class Stream, typename Char>
        void ReadString(Stream & s, std::basic_string<Char> & val)
        {
            std::basic_string<Char>::size_type len;

            ReadScalar(s, len);

            val.resize(len);

            s.Read(reinterpret_cast<uint8_t *>(const_cast<Char *>(val.data())), len * sizeof(Char));

            *(const_cast<Char *>(val.data() + len)) = 0;
        }

        template <class Stream, typename Char>
        void WriteString(Stream & s, const std::basic_string<Char> & val)
        {
            std::basic_string<Char>::size_type len = val.length();

            WriteScalar(s, len);

            s.Write(reinterpret_cast<const uint8_t *>(val.data()), len * sizeof(Char));
        }

        template <class Stream, typename T>
        inline void ReadVector(Stream & s, std::vector<T> & items)
        {
            std::vector<T>::size_type size = sizeof(T);

            s.Read(reinterpret_cast<uint8_t *>(items.data()), size * items.size());
        }

        template <class Stream, typename T>
        inline void WriteVector(Stream & s, const std::vector<T> & items)
        {
            std::vector<T>::size_type size = sizeof(T);

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
                uint8_t aggr = 0;

                for (uint8_t mask = 1; mask > 0 && i < n; ++i, mask <<= 1)
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
