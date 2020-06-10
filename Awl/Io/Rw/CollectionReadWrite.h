#pragma once

#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <type_traits>

#include "Awl/HybridSet.h"
#include "Awl/ObservableSet.h"

namespace awl::io
{
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
}