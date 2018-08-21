#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <type_traits>

namespace awl 
{
    namespace io
    {
        template <class Stream, typename T>
        inline void Read(Stream & s, T & val)
        {
            const size_t size = sizeof(T);

            s.Read(reinterpret_cast<uint8_t *>(&val), size);
        }

        template <class Stream, typename T>
        inline void Write(Stream & s, const T & val)
        {
            const size_t size = sizeof(T);

            s.Write(reinterpret_cast<const uint8_t *>(&val), size);
        }

        //Initially on Windows the type was long long, but Android build fails with it, so I changed it to int64_t, what is the difference between them?
        static_assert(std::is_same <std::chrono::nanoseconds::rep, int64_t>::value, "nanoseconds::rep is not int64_t");

        template <class Stream, class Clock, class Duration>
        inline void Read(Stream & s, std::chrono::time_point<Clock, Duration> & val)
        {
            using namespace std::chrono;

            int64_t ns_count;

            Read(s, ns_count);

            val = std::chrono::time_point<Clock, Duration>(duration_cast<Duration>(nanoseconds(ns_count)));
        }

        template <class Stream, class Clock, class Duration>
        inline void Write(Stream & s, const std::chrono::time_point<Clock, Duration> & val)
        {
            using namespace std::chrono;

            auto ns = duration_cast<nanoseconds>(val.time_since_epoch());

            int64_t ns_count = ns.count();

            Write(s, ns_count);
        }

        template <class Stream, typename Char>
        void Read(Stream & s, std::basic_string<Char> & val)
        {
            std::basic_string<Char>::size_type len;

            Read(s, len);

            val.resize(len);

            s.Read(reinterpret_cast<uint8_t *>(const_cast<Char *>(val.data())), len * sizeof(Char));

            *(const_cast<Char *>(val.data() + len)) = 0;
        }

        template <class Stream, typename Char>
        void Write(Stream & s, const std::basic_string<Char> & val)
        {
            std::basic_string<Char>::size_type len = val.length();

            Write(s, len);

            s.Write(reinterpret_cast<const uint8_t *>(val.data()), len * sizeof(Char));
        }

        template <class Stream, typename T>
        typename std::enable_if<std::is_arithmetic<T>::value && !std::is_same<T, bool>::value, void>::type ReadVector(Stream & s, std::vector<T> & v)
        {
            s.Read(reinterpret_cast<uint8_t *>(v.data()), v.size() * sizeof(T));
        }

        template <class Stream, typename T>
        typename std::enable_if<std::is_arithmetic<T>::value && !std::is_same<T, bool>::value, void>::type WriteVector(Stream & s, const std::vector<T> & v)
        {
            s.Write(reinterpret_cast<const uint8_t *>(v.data()), v.size() * sizeof(T));
        }

        template <class Stream, typename T>
        typename std::enable_if<std::is_class<T>::value, void>::type ReadVector(Stream & s, std::vector<T> & v)
        {
            for (auto & elem : v)
            {
                Read(s, elem);
            }
        }

        template <class Stream, typename T>
        typename std::enable_if<std::is_class<T>::value, void>::type WriteVector(Stream & s, const std::vector<T> & v)
        {
            for (auto & elem : v)
            {
                Write(s, elem);
            }
        }

        template <class Stream>
        void ReadVector(Stream & s, std::vector<bool> & x)
        {
            std::vector<bool>::size_type n = x.size();

            for (std::vector<bool>::size_type i = 0; i < n;)
            {
                uint8_t aggr;

                Read(s, aggr);

                for (uint8_t mask = 1; mask > 0 && i < n; ++i, mask <<= 1)
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

                Write(s, aggr);
            }
        }

        template <class Stream, typename T>
        inline void Read(Stream & s, std::vector<T> & v)
        {
            std::vector<T>::size_type size;

            Read(s, size);

            v.resize(size);

            ReadVector(s, v);
        }

        template <class Stream, typename T>
        inline void Write(Stream & s, const std::vector<T> & v)
        {
            std::vector<T>::size_type size = v.size();

            Write(s, size);

            WriteVector(s, v);
        }

        /*
        template <class Stream, typename T>
        void Serialize(Stream & s, T & val, bool is_storing)
        {
            if (is_storing)
            {
                Write(s, val);
            }
            else
            {
                Read(s, val);
            }
        }
        */
    }
}
