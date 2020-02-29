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
#include <optional>

#include "Awl/BitMap.h"
#include "Awl/HybridSet.h"
#include "Awl/ObservableSet.h"

#include "Awl/Serializable.h"
#include "Awl/Io/IoException.h"
#include "Awl/Io/RwAdapters.h"

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

        //Check if nanoseconds representation is either long or long long and its size is 8, so it can be converted to int64_t.
        static_assert(std::is_arithmetic_v<std::chrono::nanoseconds::rep> && std::is_signed_v<std::chrono::nanoseconds::rep> && sizeof(std::chrono::nanoseconds::rep) == 8);

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

            const nanoseconds ns = duration_cast<nanoseconds>(val.time_since_epoch());

            const int64_t ns_count = ns.count();

            Write(s, ns_count);
        }

        template <class Stream, typename Char>
        inline void Read(Stream & s, std::basic_string<Char> & val)
        {
            typename std::basic_string<Char>::size_type len;

            Read(s, len);

            val.resize(len);

            //There is non-const version of data() since C++ 17.
            ReadRaw(s, reinterpret_cast<uint8_t *>(val.data()), len * sizeof(Char));

            *(val.data() + len) = 0;
        }

        template <class Stream, typename Char>
        inline void Write(Stream & s, const std::basic_string<Char> & val)
        {
            typename std::basic_string<Char>::size_type len = val.length();

            Write(s, len);

            s.Write(reinterpret_cast<const uint8_t *>(val.data()), len * sizeof(Char));
        }

        template <class Stream, class Container>
        inline typename std::enable_if<std::is_arithmetic<typename Container::value_type>::value && !std::is_same<typename Container::value_type, bool>::value, void>::type
            ReadVector(Stream & s, Container & v)
        {
            ReadRaw(s, reinterpret_cast<uint8_t *>(v.data()), v.size() * sizeof(typename Container::value_type));
        }

        template <class Stream, class Container>
        inline typename std::enable_if<std::is_arithmetic<typename Container::value_type>::value && !std::is_same<typename Container::value_type, bool>::value, void>::type
            WriteVector(Stream & s, const Container & v)
        {
            s.Write(reinterpret_cast<const uint8_t *>(v.data()), v.size() * sizeof(typename Container::value_type));
        }

        //vector<string>, for example.
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

            for (typename Container::size_type i = 0; i < n;)
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

            for (typename Container::size_type i = 0; i < n;)
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

        //Looks like std::bitset<N> does not have value_type, so we use BitSetAdapter
        template <class Stream, std::size_t N>
        inline void Read(Stream & s, std::bitset<N> & v)
        {
            adapters::BitSetAdapter<std::bitset<N>> a(v);
            
            ReadVector(s, a);
        }

        template <class Stream, std::size_t N>
        inline void Write(Stream & s, const std::bitset<N> & v)
        {
            const adapters::BitSetAdapter<const std::bitset<N>> a(v);

            WriteVector(s, a);
        }

        template <class Stream, typename Enum, typename std::underlying_type<Enum>::type N>
        inline void Read(Stream & s, bitmap<Enum, N> & v)
        {
            adapters::BitMapAdapter<bitmap<Enum, N>> a(v);

            ReadVector(s, a);
        }

        template <class Stream, typename Enum, typename std::underlying_type<Enum>::type N>
        inline void Write(Stream & s, const bitmap<Enum, N> & v)
        {
            const adapters::BitMapAdapter<const bitmap<Enum, N>> a(v);

            WriteVector(s, a);
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

        //There is a separate function for reading a map because the first pair type is const (std::pair<const Key, T>):
        template <class Stream, typename Coll>
        void ReadMap(Stream & s, Coll & coll)
        {
            size_t count;

            Read(s, count);

            for (size_t i = 0; i < count; ++i)
            {
                typename Coll::key_type key;
                Read(s, key);

                typename Coll::mapped_type value;
                Read(s, value);

                coll.insert(std::make_pair(key, value));
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
        inline void Read(Stream & s, std::set<T, Compare, Alloc> & coll)
        {
            ReadCollection(s, coll);
        }

        template <class Stream, class T, class Compare, class Alloc>
        inline void Write(Stream & s, const std::set<T, Compare, Alloc> &coll)
        {
            WriteCollection(s, coll);
        }

        template <class Stream, class T, class Compare, class Alloc>
        inline void Read(Stream & s, hybrid_set<T, Compare, Alloc> & coll)
        {
            ReadCollection(s, coll);
        }

        template <class Stream, class T, class Compare, class Alloc>
        inline void Write(Stream & s, const hybrid_set<T, Compare, Alloc> &coll)
        {
            WriteCollection(s, coll);
        }

        template <class Stream, class T, class Compare, class Alloc>
        inline void Read(Stream & s, observable_set<T, Compare, Alloc> & coll)
        {
            ReadCollection(s, coll);
        }

        template <class Stream, class T, class Compare, class Alloc>
        inline void Write(Stream & s, const observable_set<T, Compare, Alloc> &coll)
        {
            WriteCollection(s, coll);
        }

        template<class Stream, class T, class Hash, class KeyEqual, class Allocator>
        inline void Read(Stream & s, std::unordered_set<T, Hash, KeyEqual, Allocator> & coll)
        {
            ReadCollection(s, coll);
        }

        template<class Stream, class T, class Hash, class KeyEqual, class Allocator>
        inline void Write(Stream & s, const std::unordered_set<T, Hash, KeyEqual, Allocator> &coll)
        {
            WriteCollection(s, coll);
        }

        template <class Stream, class Key, class T, class Compare, class Alloc>
        inline void Read(Stream & s, std::map<Key, T, Compare, Alloc> & coll)
        {
            ReadMap(s, coll);
        }

        template <class Stream, class Key, class T, class Compare, class Alloc>
        inline void Write(Stream & s, const std::map<Key, T, Compare, Alloc> &coll)
        {
            WriteCollection(s, coll);
        }

        template<class Stream, class Key, class T, class Hash, class KeyEqual, class Allocator>
        inline void Read(Stream & s, std::unordered_map<Key, T, Hash, KeyEqual, Allocator> & coll)
        {
            ReadMap(s, coll);
        }

        template<class Stream, class T, class Key, class Hash, class KeyEqual, class Allocator>
        inline void Write(Stream & s, const std::unordered_map<Key, T, Hash, KeyEqual, Allocator> &coll)
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
        inline typename std::enable_if<is_tuplizable_v<T>, void>::type Read(Stream & s, T & val)
        {
            Read(s, object_as_tuple(val));
        }

        template <class Stream, typename T>
        inline typename std::enable_if<is_tuplizable_v<T>, void>::type Write(Stream & s, const T & val)
        {
            Write(s, object_as_tuple(val));
        }

        template <class Stream, typename T>
        inline void Read(Stream & s, std::optional<T>& opt_val)
        {
            bool has_value;

            Read(s, has_value);

            if (has_value)
            {
                T val;

                Read(s, val);

                opt_val = std::move(val);
            }
        }

        template <class Stream, typename T>
        inline void Write(Stream & s, const std::optional<T>& opt_val)
        {
            const bool has_value = opt_val.has_value();

            Write(s, has_value);

            if (has_value)
            {
                Write(s, opt_val.value());
            }
        }
    }
}
