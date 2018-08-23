#pragma once

#include <string>
#include <vector>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <chrono>
#include <type_traits>
#include <tuple>
#include <utility> 

#include "Awl/TupleHelpers.h"

namespace awl
{
    namespace io
    {
        template <class Stream, typename T>
        inline typename std::enable_if<std::is_arithmetic<T>::value && !std::is_same<T, bool>::value, void>::type Read(Stream & s, T & val)
        {
            const size_t size = sizeof(T);

            s.Read(reinterpret_cast<uint8_t *>(&val), size);
        }

        //Scalar types are passed by value but not by const reference.
        template <class Stream, typename T>
        inline typename std::enable_if<std::is_arithmetic<T>::value && !std::is_same<T, bool>::value, void>::type Write(Stream & s, T val)
        {
            const size_t size = sizeof(T);

            s.Write(reinterpret_cast<const uint8_t *>(&val), size);
        }

        //sizeof(bool) is implementation-defined and it is not required to be 1.

        template <class Stream>
        inline void Read(Stream & s, bool & b)
        {
            uint8_t val;

            Read(s, val);

            b = val != 0;
        }

        template <class Stream>
        inline void Write(Stream & s, bool b)
        {
            uint8_t val = b ? 1 : 0;

            Write(s, val);
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
        inline void Write(Stream & s, std::chrono::time_point<Clock, Duration> val)
        {
            using namespace std::chrono;

            auto ns = duration_cast<nanoseconds>(val.time_since_epoch());

            int64_t ns_count = ns.count();

            Write(s, ns_count);
        }

        template <class Stream, typename Char>
        void Read(Stream & s, std::basic_string<Char> & val)
        {
            typename std::basic_string<Char>::size_type len;

            Read(s, len);

            val.resize(len);

            s.Read(reinterpret_cast<uint8_t *>(const_cast<Char *>(val.data())), len * sizeof(Char));

            *(const_cast<Char *>(val.data() + len)) = 0;
        }

        template <class Stream, typename Char>
        void Write(Stream & s, const std::basic_string<Char> & val)
        {
            typename std::basic_string<Char>::size_type len = val.length();

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
            typename std::vector<bool>::size_type n = x.size();

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
            typename std::vector<bool>::size_type n = x.size();

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
            typename std::vector<T>::size_type size;

            Read(s, size);

            v.resize(size);

            ReadVector(s, v);
        }

        template <class Stream, typename T>
        inline void Write(Stream & s, const std::vector<T> & v)
        {
            typename std::vector<T>::size_type size = v.size();

            Write(s, size);

            WriteVector(s, v);
        }

        template <class T>
        struct remove_const
        {
            typedef T type;
        };

        template <class T>
        struct remove_const<const T>
        {
            typedef T type;
        };

        template <class Stream, class First, class Second>
        inline void Read(Stream & s, std::pair<First, Second> & val)
        {
            Read(s, const_cast<typename remove_const<First>::type &>(val.first));
            Read(s, val.second);
        }

        template <class Stream, class First, class Second>
        inline void Write(Stream & s, const std::pair<First, Second> & val)
        {
            Write(s, val.first);
            Write(s, val.second);
        }

        template <class Stream, typename Coll>
        void ReadCollection(Stream & s, Coll & coll)
        {
            size_t count;

            Read(s, count);

            for (size_t i = 0; i < count; ++i)
            {
                typename Coll::value_type elem;

                Read(s, elem);

                coll.insert(elem);
            }
        }

        template <class Stream, typename Coll>
        void WriteCollection(Stream & s, const Coll & coll)
        {
            size_t count = coll.size();

            Write(s, count);

            for (auto & elem : coll)
            {
                Write(s, elem);
            }
        }

        template <class Stream, class T, class Compare, class Alloc>
        void Read(Stream & s, std::set<T, Compare, Alloc> & coll)
        {
            ReadCollection(s, coll);
        }

        template <class Stream, class T, class Compare, class Alloc>
        void Write(Stream & s, const std::set<T, Compare, Alloc> &coll)
        {
            WriteCollection(s, coll);
        }

        template<class Stream, class T, class Hash, class KeyEqual, class Allocator>
        void Read(Stream & s, std::unordered_set<T, Hash, KeyEqual, Allocator> & coll)
        {
            ReadCollection(s, coll);
        }

        template<class Stream, class T, class Hash, class KeyEqual, class Allocator>
        void Write(Stream & s, const std::unordered_set<T, Hash, KeyEqual, Allocator> &coll)
        {
            WriteCollection(s, coll);
        }

        template <class Stream, class Key, class T, class Compare, class Alloc>
        void Read(Stream & s, std::map<Key, T, Compare, Alloc> & coll)
        {
            ReadCollection(s, coll);
        }

        template <class Stream, class Key, class T, class Compare, class Alloc>
        void Write(Stream & s, const std::map<Key, T, Compare, Alloc> &coll)
        {
            WriteCollection(s, coll);
        }

        template<class Stream, class Key, class T, class Hash, class KeyEqual, class Allocator>
        void Read(Stream & s, std::unordered_map<Key, T, Hash, KeyEqual, Allocator> & coll)
        {
            ReadCollection(s, coll);
        }

        template<class Stream, class T, class Key, class Hash, class KeyEqual, class Allocator>
        void Write(Stream & s, const std::unordered_map<Key, T, Hash, KeyEqual, Allocator> &coll)
        {
            WriteCollection(s, coll);
        }

        template<class Stream, std::size_t I = 0, typename... Tp>
        inline typename std::enable_if<(I == sizeof...(Tp)), void>::type ReadEach(Stream &, std::tuple<Tp...> &) // Unused arguments are given no names.
        {
        }

        template<class Stream, std::size_t I = 0, typename... Tp>
        inline typename std::enable_if<(I < sizeof...(Tp)), void>::type ReadEach(Stream & s, std::tuple<Tp...>& t)
        {
            Read(s, std::get<I>(t));
            ReadEach<Stream, I + 1, Tp...>(s, t);
        }

        template<class Stream, typename ... Fields>
        void Read(Stream & s, std::tuple<Fields...> & val)
        {
            ReadEach(s, val);
        }

        template<class Stream, std::size_t I = 0, typename... Tp>
        inline typename std::enable_if<(I == sizeof...(Tp)), void>::type WriteEach(Stream &, const std::tuple<Tp...> &) // Unused arguments are given no names.
        {
        }

        template<class Stream, std::size_t I = 0, typename... Tp>
        inline typename std::enable_if<(I < sizeof...(Tp)), void>::type WriteEach(Stream & s, const std::tuple<Tp...>& t)
        {
            Write(s, std::get<I>(t));
            WriteEach<Stream, I + 1, Tp...>(s, t);
        }

        template<class Stream, typename ... Fields>
        void Write(Stream & s, const std::tuple<Fields...> & val)
        {
            WriteEach(s, val);
        }

        template <class T>
        auto class_as_tuple(T & val)
        {
            return val.as_tuple();
        }
        
        template <class T>
        auto class_as_const_tuple(const T & val)
        {
            //Remove const and then make it const again.
            const auto & tuple_val = class_as_tuple(const_cast<T &>(val));

            return tuple_val;
        }

        template <class Stream, typename T>
        typename std::enable_if<std::is_class<T>::value, void>::type Read(Stream & s, T & val)
        {
            Read(s, class_as_tuple(val));
        }

        template <class Stream, typename T>
        typename std::enable_if<std::is_class<T>::value, void>::type Write(Stream & s, const T & val)
        {
            Write(s, class_as_const_tuple(val));
        }

        template <class T>
        bool objects_equal(const T & left, const T & right)
        {
            return class_as_const_tuple(left) == class_as_const_tuple(right);
        }

        template <class T>
        bool objects_less(const T & left, const T & right)
        {
            return class_as_const_tuple(left) < class_as_const_tuple(right);
        }
    }
}
