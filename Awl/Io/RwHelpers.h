#pragma once

#include <string>
#include <vector>
#include <array>
#include <bitset>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <chrono>
#include <type_traits>
#include <tuple>
#include <utility> 

#include "Awl/Serializable.h"
#include "Awl/Io/IoException.h"

namespace awl
{
    namespace io
    {
        //The benefit of having Stream template parameter in all Read/Write methods is that Stream::Read and Stream::Write functions
        //can be called as non-virtual and even as constexpr if the final Stream type is known at compile time.

        template <class Stream>
        inline void ReadRaw(Stream & s, uint8_t * buffer, size_t count)
        {
            const size_t actually_read = s.Read(buffer, count);

            assert(actually_read <= count);

            if (actually_read < count)
            {
                throw EndOfFileException(count, actually_read);
            }
        }
        
        template <class Stream, typename T>
        inline typename std::enable_if<std::is_arithmetic<T>::value && !std::is_same<T, bool>::value, void>::type Read(Stream & s, T & val)
        {
            const size_t size = sizeof(T);

            ReadRaw(s, reinterpret_cast<uint8_t *>(&val), size);
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

            ReadRaw(s, reinterpret_cast<uint8_t *>(const_cast<Char *>(val.data())), len * sizeof(Char));

            *(const_cast<Char *>(val.data() + len)) = 0;
        }

        template <class Stream, typename Char>
        void Write(Stream & s, const std::basic_string<Char> & val)
        {
            typename std::basic_string<Char>::size_type len = val.length();

            Write(s, len);

            s.Write(reinterpret_cast<const uint8_t *>(val.data()), len * sizeof(Char));
        }

        template <class Stream, class Container>
        typename std::enable_if<std::is_arithmetic<typename Container::value_type>::value && !std::is_same<typename Container::value_type, bool>::value, void>::type 
            ReadVector(Stream & s, Container & v)
        {
            ReadRaw(s, reinterpret_cast<uint8_t *>(v.data()), v.size() * sizeof(typename Container::value_type));
        }

        template <class Stream, class Container>
        typename std::enable_if<std::is_arithmetic<typename Container::value_type>::value && !std::is_same<typename Container::value_type, bool>::value, void>::type 
            WriteVector(Stream & s, const Container & v)
        {
            s.Write(reinterpret_cast<const uint8_t *>(v.data()), v.size() * sizeof(typename Container::value_type));
        }

        template <class Stream, class Container>
        typename std::enable_if<std::is_class<typename Container::value_type>::value, void>::type 
            ReadVector(Stream & s, Container & v)
        {
            for (auto & elem : v)
            {
                Read(s, elem);
            }
        }

        template <class Stream, class Container>
        typename std::enable_if<std::is_class<typename Container::value_type>::value, void>::type 
            WriteVector(Stream & s, const Container & v)
        {
            for (const auto & elem : v)
            {
                Write(s, elem);
            }
        }

        template <class Stream, class Container>
        typename std::enable_if<std::is_same<typename Container::value_type, bool>::value, void>::type
            ReadVector(Stream & s, Container & x)
        {
            typename Container::size_type n = x.size();

            for (Container::size_type i = 0; i < n;)
            {
                uint8_t aggr;

                Read(s, aggr);

                for (uint8_t mask = 1; mask > 0 && i < n; ++i, mask <<= 1)
                {
                    x.at(i) = (aggr & mask) != 0;
                }
            }
        }

        template <class Stream, class Container>
        typename std::enable_if<std::is_same<typename Container::value_type, bool>::value, void>::type
            WriteVector(Stream & s, const Container & x)
        {
            typename Container::size_type n = x.size();

            for (Container::size_type i = 0; i < n;)
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

        //std::array has no specialization for bool type, but we save std::array<bool, N> in the same format as std::vector<bool>.
        template <class Stream, typename T, std::size_t N>
        inline void Read(Stream & s, std::array<T, N> & v)
        {
            ReadVector(s, v);
        }

        template <class Stream, typename T, std::size_t N>
        inline void Write(Stream & s, const std::array<T, N> & v)
        {
            WriteVector(s, v);
        }

        //looks like std::bitset<N> does not have value_type
        //we save std::bitset<N> in the same format as std::vector<bool>.
        //template <class Stream, std::size_t N>
        //inline void Read(Stream & s, std::bitset<N> & v)
        //{
        //    ReadVector(s, v);
        //}

        //template <class Stream, std::size_t N>
        //inline void Write(Stream & s, const std::bitset<N> & v)
        //{
        //    WriteVector(s, v);
        //}

        template <class Stream, class First, class Second>
        inline void Read(Stream & s, std::pair<First, Second> & val)
        {
            Read(s, const_cast<typename std::remove_const<First>::type &>(val.first));
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

#if AWL_CPPSTD >= 17

        //Implementing Read/WriteEach with fold expressions.

        template<class Stream, typename ... Fields>
        inline void ReadEach(Stream & s, std::tuple<Fields& ...> val)
        {
            for_each(val, [&s](auto& field) { Read(s, field); });
        }

        template<class Stream, typename ... Fields>
        inline void WriteEach(Stream & s, const std::tuple<Fields& ...> & val)
        {
            for_each(val, [&s](auto& field) { Write(s, field); });
        }

#else
        //Implementing Read/WriteEach with recursive templates.

        template<class Stream, std::size_t I = 0, typename... Tp>
        inline typename std::enable_if<(I == sizeof...(Tp)), void>::type ReadEach(Stream &, std::tuple<Tp...> &)
        {
        }

        template<class Stream, std::size_t I = 0, typename... Tp>
        inline typename std::enable_if<(I < sizeof...(Tp)), void>::type ReadEach(Stream & s, std::tuple<Tp...>& t)
        {
            Read(s, std::get<I>(t));
            ReadEach<Stream, I + 1, Tp...>(s, t);
        }

        template<class Stream, std::size_t I = 0, typename... Tp>
        inline typename std::enable_if<(I == sizeof...(Tp)), void>::type WriteEach(Stream &, const std::tuple<Tp...> &)
        {
        }

        template<class Stream, std::size_t I = 0, typename... Tp>
        inline typename std::enable_if<(I < sizeof...(Tp)), void>::type WriteEach(Stream & s, const std::tuple<Tp...>& t)
        {
            Write(s, std::get<I>(t));
            WriteEach<Stream, I + 1, Tp...>(s, t);
        }

#endif
        //A tuple of references is passed by value.
        template<class Stream, typename ... Fields>
        inline void Read(Stream & s, std::tuple<Fields& ...> val)
        {
            ReadEach(s, val);
        }

        //A tuple of values is passed by reference. Cannot figure out why this does not compile with VC2017.
        //template<class Stream, typename ... Fields>
        //void Read(Stream & s, std::tuple<Fields ...> & val)
        //{
        //    ReadEach(s, val);
        //}

        template<class Stream, typename ... Fields>
        inline void Write(Stream & s, const std::tuple<Fields& ...> & val)
        {
            WriteEach(s, val);
        }

        template <class Stream, typename T>
        inline typename std::enable_if<std::is_class<T>::value, void>::type Read(Stream & s, T & val)
        {
            Read(s, object_as_tuple(val));
        }

        template <class Stream, typename T>
        inline typename std::enable_if<std::is_class<T>::value, void>::type Write(Stream & s, const T & val)
        {
            Write(s, object_as_tuple(val));
        }
    }
}
