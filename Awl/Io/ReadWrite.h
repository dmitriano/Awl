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
#include "Awl/Stringizable.h"
#include "Awl/Io/IoException.h"
#include "Awl/Io/RwAdapters.h"

namespace awl::io
{
    class FakeContext
    {
    };

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
        
    template <class Stream, typename T, class Context = FakeContext>
    inline typename std::enable_if<std::is_arithmetic<T>::value && !std::is_same<T, bool>::value, void>::type 
        Read(Stream & s, T & val, const Context & ctx = {})
    {
        static_cast<void>(ctx);

        const size_t size = sizeof(T);

        ReadRaw(s, reinterpret_cast<uint8_t *>(&val), size);
    }

    //Scalar types are passed by value but not by const reference.
    template <class Stream, typename T, class Context = FakeContext>
    inline typename std::enable_if<std::is_arithmetic<T>::value && !std::is_same<T, bool>::value, void>::type 
        Write(Stream & s, T val, const Context & ctx = {})
    {
        static_cast<void>(ctx);
        
        const size_t size = sizeof(T);

        s.Write(reinterpret_cast<const uint8_t *>(&val), size);
    }

    //sizeof(bool) is implementation-defined and it is not required to be 1.

    template <class Stream, class Context = FakeContext>
    inline void Read(Stream & s, bool & b, const Context & ctx = {})
    {
        uint8_t val;

        Read(s, val, ctx);

        b = val != 0;
    }

    template <class Stream, class Context = FakeContext>
    inline void Write(Stream & s, bool b, const Context & ctx = {})
    {
        uint8_t val = b ? 1 : 0;

        Write(s, val, ctx);
    }

    //Check if nanoseconds representation is either long or long long and its size is 8, so it can be converted to int64_t.
    static_assert(std::is_arithmetic_v<std::chrono::nanoseconds::rep> && std::is_signed_v<std::chrono::nanoseconds::rep> && sizeof(std::chrono::nanoseconds::rep) == 8);

    template <class Stream, class Clock, class Duration, class Context = FakeContext>
    inline void Read(Stream & s, std::chrono::time_point<Clock, Duration> & val, const Context & ctx = {})
    {
        using namespace std::chrono;

        int64_t ns_count;

        Read(s, ns_count, ctx);

        val = std::chrono::time_point<Clock, Duration>(duration_cast<Duration>(nanoseconds(ns_count)));
    }

    template <class Stream, class Clock, class Duration, class Context = FakeContext>
    inline void Write(Stream & s, std::chrono::time_point<Clock, Duration> val, const Context & ctx = {})
    {
        using namespace std::chrono;

        const nanoseconds ns = duration_cast<nanoseconds>(val.time_since_epoch());

        const int64_t ns_count = ns.count();

        Write(s, ns_count, ctx);
    }

    template<
        class Stream,
        class Char,
        class Traits = std::char_traits<Char>,
        class Allocator = std::allocator<Char>,
        class Context = FakeContext
    >
    inline void Read(Stream & s, std::basic_string<Char, Traits, Allocator> & val, const Context & ctx = {})
    {
        typename std::basic_string<Char>::size_type len;

        Read(s, len, ctx);

        val.resize(len);

        //There is non-const version of data() since C++ 17.
        ReadRaw(s, reinterpret_cast<uint8_t *>(val.data()), len * sizeof(Char));

        *(val.data() + len) = 0;
    }

    template<
        class Stream,
        class Char,
        class Traits = std::char_traits<Char>,
        class Allocator = std::allocator<Char>,
        class Context = FakeContext
    >
    inline void Write(Stream & s, const std::basic_string<Char, Traits, Allocator> & val, const Context & ctx = {})
    {
        typename std::basic_string<Char>::size_type len = val.length();

        Write(s, len, ctx);

        s.Write(reinterpret_cast<const uint8_t *>(val.data()), len * sizeof(Char));
    }

    template <class Stream, class Container, class Context = FakeContext>
    inline typename std::enable_if<std::is_arithmetic<typename Container::value_type>::value && !std::is_same<typename Container::value_type, bool>::value, void>::type
        ReadVector(Stream & s, Container & v, const Context & ctx = {})
    {
        static_cast<void>(ctx);
        ReadRaw(s, reinterpret_cast<uint8_t *>(v.data()), v.size() * sizeof(typename Container::value_type));
    }

    template <class Stream, class Container, class Context = FakeContext>
    inline typename std::enable_if<std::is_arithmetic<typename Container::value_type>::value && !std::is_same<typename Container::value_type, bool>::value, void>::type
        WriteVector(Stream & s, const Container & v, const Context & ctx = {})
    {
        static_cast<void>(ctx);
        s.Write(reinterpret_cast<const uint8_t *>(v.data()), v.size() * sizeof(typename Container::value_type));
    }

    //vector<string>, for example.
    template <class Stream, class Container, class Context = FakeContext>
    typename std::enable_if<std::is_class<typename Container::value_type>::value, void>::type 
        ReadVector(Stream & s, Container & v, const Context & ctx = {})
    {
        for (auto & elem : v)
        {
            Read(s, elem, ctx);
        }
    }

    template <class Stream, class Container, class Context = FakeContext>
    typename std::enable_if<std::is_class<typename Container::value_type>::value, void>::type 
        WriteVector(Stream & s, const Container & v, const Context & ctx = {})
    {
        for (const auto & elem : v)
        {
            Write(s, elem, ctx);
        }
    }

    template <class Stream, class Container, class Context = FakeContext>
    typename std::enable_if<std::is_same<typename Container::value_type, bool>::value, void>::type
        ReadVector(Stream & s, Container & x, const Context & ctx = {})
    {
        typename Container::size_type n = x.size();

        for (typename Container::size_type i = 0; i < n;)
        {
            uint8_t aggr;

            Read(s, aggr, ctx);

            for (uint8_t mask = 1; mask > 0 && i < n; ++i, mask <<= 1)
            {
                x.at(i) = (aggr & mask) != 0;
            }
        }
    }

    template <class Stream, class Container, class Context = FakeContext>
    typename std::enable_if<std::is_same<typename Container::value_type, bool>::value, void>::type
        WriteVector(Stream & s, const Container & x, const Context & ctx = {})
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

            Write(s, aggr, ctx);
        }
    }

    template <class Stream, class T, class Allocator = std::allocator<T>, class Context = FakeContext>
    inline void Read(Stream & s, std::vector<T, Allocator> & v, const Context & ctx = {})
    {
        typename std::vector<T, Allocator>::size_type size;

        Read(s, size, ctx);

        v.resize(size);

        ReadVector(s, v, ctx);
    }

    template <class Stream, class T, class Allocator = std::allocator<T>, class Context = FakeContext>
    inline void Write(Stream & s, const std::vector<T, Allocator> & v, const Context & ctx = {})
    {
        typename std::vector<T, Allocator>::size_type size = v.size();

        Write(s, size, ctx);

        WriteVector(s, v, ctx);
    }

    //std::array has no specialization for bool type, but we save std::array<bool, N> in the same format as std::vector<bool>.
    template <class Stream, typename T, std::size_t N, class Context = FakeContext>
    inline void Read(Stream & s, std::array<T, N> & v, const Context & ctx = {})
    {
        ReadVector(s, v, ctx);
    }

    template <class Stream, typename T, std::size_t N, class Context = FakeContext>
    inline void Write(Stream & s, const std::array<T, N> & v, const Context & ctx = {})
    {
        WriteVector(s, v, ctx);
    }

    //Looks like std::bitset<N> does not have value_type, so we use BitSetAdapter
    template <class Stream, std::size_t N, class Context = FakeContext>
    inline void Read(Stream & s, std::bitset<N> & v, const Context & ctx = {})
    {
        adapters::BitSetAdapter<std::bitset<N>> a(v);
            
        ReadVector(s, a, ctx);
    }

    template <class Stream, std::size_t N, class Context = FakeContext>
    inline void Write(Stream & s, const std::bitset<N> & v, const Context & ctx = {})
    {
        const adapters::BitSetAdapter<const std::bitset<N>> a(v);

        WriteVector(s, a, ctx);
    }

    template <class Stream, typename Enum, typename std::underlying_type<Enum>::type N, class Context = FakeContext>
    inline void Read(Stream & s, bitmap<Enum, N> & v, const Context & ctx = {})
    {
        adapters::BitMapAdapter<bitmap<Enum, N>> a(v);

        ReadVector(s, a, ctx);
    }

    template <class Stream, typename Enum, typename std::underlying_type<Enum>::type N, class Context = FakeContext>
    inline void Write(Stream & s, const bitmap<Enum, N> & v, const Context & ctx = {})
    {
        const adapters::BitMapAdapter<const bitmap<Enum, N>> a(v);

        WriteVector(s, a, ctx);
    }

    template <class Stream, class First, class Second, class Context = FakeContext>
    inline void Write(Stream & s, const std::pair<First, Second> & val, const Context & ctx = {})
    {
        Write(s, val.first, ctx);
        Write(s, val.second, ctx);
    }

    template <class Stream, typename Coll, class Context = FakeContext>
    void ReadCollection(Stream & s, Coll & coll, const Context & ctx = {})
    {
        size_t count;

        Read(s, count, ctx);

        for (size_t i = 0; i < count; ++i)
        {
            typename Coll::value_type elem;

            Read(s, elem, ctx);

            coll.insert(elem);
        }
    }

    //There is a separate function for reading a map because the first pair type is const (std::pair<const Key, T>):
    template <class Stream, typename Coll, class Context = FakeContext>
    void ReadMap(Stream & s, Coll & coll, const Context & ctx = {})
    {
        size_t count;

        Read(s, count, ctx);

        for (size_t i = 0; i < count; ++i)
        {
            typename Coll::key_type key;
            Read(s, key, ctx);

            typename Coll::mapped_type value;
            Read(s, value, ctx);

            coll.insert(std::make_pair(key, value));
        }
    }

    template <class Stream, typename Coll, class Context = FakeContext>
    void WriteCollection(Stream & s, const Coll & coll, const Context & ctx = {})
    {
        size_t count = coll.size();

        Write(s, count, ctx);

        for (auto & elem : coll)
        {
            Write(s, elem, ctx);
        }
    }

    template <class Stream, class T, class Compare, class Alloc, class Context = FakeContext>
    inline void Read(Stream & s, std::set<T, Compare, Alloc> & coll, const Context & ctx = {})
    {
        ReadCollection(s, coll, ctx);
    }

    template <class Stream, class T, class Compare, class Alloc, class Context = FakeContext>
    inline void Write(Stream & s, const std::set<T, Compare, Alloc> &coll, const Context & ctx = {})
    {
        WriteCollection(s, coll, ctx);
    }

    template <class Stream, class T, class Compare, class Alloc, class Context = FakeContext>
    inline void Read(Stream & s, hybrid_set<T, Compare, Alloc> & coll, const Context & ctx = {})
    {
        ReadCollection(s, coll, ctx);
    }

    template <class Stream, class T, class Compare, class Alloc, class Context = FakeContext>
    inline void Write(Stream & s, const hybrid_set<T, Compare, Alloc> &coll, const Context & ctx = {})
    {
        WriteCollection(s, coll, ctx);
    }

    template <class Stream, class T, class Compare, class Alloc, class Context = FakeContext>
    inline void Read(Stream & s, observable_set<T, Compare, Alloc> & coll, const Context & ctx = {})
    {
        ReadCollection(s, coll, ctx);
    }

    template <class Stream, class T, class Compare, class Alloc, class Context = FakeContext>
    inline void Write(Stream & s, const observable_set<T, Compare, Alloc> &coll, const Context & ctx = {})
    {
        WriteCollection(s, coll, ctx);
    }

    template<class Stream, class T, class Hash, class KeyEqual, class Allocator, class Context = FakeContext>
    inline void Read(Stream & s, std::unordered_set<T, Hash, KeyEqual, Allocator> & coll, const Context & ctx = {})
    {
        ReadCollection(s, coll, ctx);
    }

    template<class Stream, class T, class Hash, class KeyEqual, class Allocator, class Context = FakeContext>
    inline void Write(Stream & s, const std::unordered_set<T, Hash, KeyEqual, Allocator> &coll, const Context & ctx = {})
    {
        WriteCollection(s, coll, ctx);
    }

    template <class Stream, class Key, class T, class Compare, class Alloc, class Context = FakeContext>
    inline void Read(Stream & s, std::map<Key, T, Compare, Alloc> & coll, const Context & ctx = {})
    {
        ReadMap(s, coll, ctx);
    }

    template <class Stream, class Key, class T, class Compare, class Alloc, class Context = FakeContext>
    inline void Write(Stream & s, const std::map<Key, T, Compare, Alloc> &coll, const Context & ctx = {})
    {
        WriteCollection(s, coll, ctx);
    }

    template<class Stream, class Key, class T, class Hash, class KeyEqual, class Allocator, class Context = FakeContext>
    inline void Read(Stream & s, std::unordered_map<Key, T, Hash, KeyEqual, Allocator> & coll, const Context & ctx = {})
    {
        ReadMap(s, coll, ctx);
    }

    template<class Stream, class T, class Key, class Hash, class KeyEqual, class Allocator, class Context = FakeContext>
    inline void Write(Stream & s, const std::unordered_map<Key, T, Hash, KeyEqual, Allocator> &coll, const Context & ctx = {})
    {
        WriteCollection(s, coll, ctx);
    }

    //Implementing Read/WriteEach with fold expressions.

    template<class Stream, typename ... Fields, class Context = FakeContext>
    inline void ReadEach(Stream & s, std::tuple<Fields& ...> val, const Context & ctx = {})
    {
        for_each(val, [&s, &ctx](auto& field) { Read(s, field, ctx); });
    }

    template<class Stream, typename ... Fields, class Context = FakeContext>
    inline void WriteEach(Stream & s, const std::tuple<Fields& ...> & val, const Context & ctx = {})
    {
        for_each(val, [&s, &ctx](auto& field) { Write(s, field, ctx); });
    }

    //A tuple of references is passed by value.
    template<class Stream, typename ... Fields, class Context = FakeContext>
    inline void Read(Stream & s, std::tuple<Fields& ...> val, const Context & ctx = {})
    {
        ReadEach(s, val, ctx);
    }

    //A tuple of values is passed by reference. Cannot figure out why this does not compile with VC2017.
    //template<class Stream, typename ... Fields>
    //void Read(Stream & s, std::tuple<Fields ...> & val)
    //{
    //    ReadEach(s, val);
    //}

    template<class Stream, typename ... Fields, class Context = FakeContext>
    inline void Write(Stream & s, const std::tuple<Fields& ...> & val, const Context & ctx = {})
    {
        WriteEach(s, val, ctx);
    }

    template <class Stream, typename T, class Context = FakeContext>
    inline typename std::enable_if<is_tuplizable_v<T>, void>::type Read(Stream & s, T & val, const Context & ctx = {})
    {
        if constexpr (std::is_same_v<Context, FakeContext>)
        {
            Read(s, object_as_tuple(val), ctx);
        }
        else
        {
            ctx.ReadV(s, val);
        }
    }

    template <class Stream, typename T, class Context = FakeContext>
    inline typename std::enable_if<is_tuplizable_v<T>, void>::type Write(Stream & s, const T & val, const Context & ctx = {})
    {
        if constexpr (std::is_same_v<Context, FakeContext>)
        {
            Write(s, object_as_tuple(val), ctx);
        }
        else
        {
            ctx.WriteV(s, val);
        }
    }

    template <class Stream, typename T, class Context = FakeContext>
    inline void Read(Stream & s, std::optional<T>& opt_val, const Context & ctx = {})
    {
        bool has_value;

        Read(s, has_value, ctx);

        if (has_value)
        {
            T val;

            Read(s, val, ctx);

            opt_val = std::move(val);
        }
    }

    template <class Stream, typename T, class Context = FakeContext>
    inline void Write(Stream & s, const std::optional<T>& opt_val, const Context & ctx = {})
    {
        const bool has_value = opt_val.has_value();

        Write(s, has_value, ctx);

        if (has_value)
        {
            Write(s, opt_val.value(), ctx);
        }
    }
}
