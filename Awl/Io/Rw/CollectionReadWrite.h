/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/VectorSet.h"
#include "Awl/ObservableSet.h"
#include "Awl/Ring.h"
#include "Awl/Io/Rw/RwAdapters.h"

#include <deque>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <type_traits>
namespace awl::io
{
    template <class Stream, class First, class Second, class Context = FakeContext>
        requires sequential_output_stream<Stream>
    void write(Stream & s, const std::pair<First, Second> & val, const Context & ctx = {})
    {
        write(s, val.first, ctx);
        write(s, val.second, ctx);
    }

    template <class Stream, typename Coll, class Context = FakeContext>
        requires sequential_input_stream<Stream>
    void readCollection(Stream & s, Coll & coll, const Context & ctx = {})
    {
        size_t count;

        read(s, count, ctx);

        for (size_t i = 0; i < count; ++i)
        {
            typename Coll::value_type elem;

            read(s, elem, ctx);

            coll.insert(elem);
        }
    }

    template <class Stream, typename Coll, class Context = FakeContext>
        requires sequential_input_stream<Stream>
    void readDeque(Stream& s, Coll& coll, const Context& ctx = {})
    {
        size_t count;

        read(s, count, ctx);

        for (size_t i = 0; i < count; ++i)
        {
            typename Coll::value_type elem;

            read(s, elem, ctx);

            coll.push_back(elem);
        }
    }

    //There is a separate function for reading a map because the first pair type is const (std::pair<const Key, T>):
    template <class Stream, typename Coll, class Context = FakeContext>
        requires sequential_input_stream<Stream>
    void readMap(Stream & s, Coll & coll, const Context & ctx = {})
    {
        size_t count;

        read(s, count, ctx);

        for (size_t i = 0; i < count; ++i)
        {
            typename Coll::key_type key;
            read(s, key, ctx);

            typename Coll::mapped_type value;
            read(s, value, ctx);

            coll.insert(std::make_pair(key, value));
        }
    }

    template <class Stream, typename Coll, class Context = FakeContext>
        requires sequential_output_stream<Stream>
    void writeCollection(Stream & s, const Coll & coll, const Context & ctx = {})
    {
        size_t count = coll.size();

        write(s, count, ctx);

        for (auto & elem : coll)
        {
            write(s, elem, ctx);
        }
    }

    template <class Stream, class T, class Alloc, class Context = FakeContext>
        requires sequential_input_stream<Stream>
    void read(Stream& s, std::deque<T, Alloc>& coll, const Context& ctx = {})
    {
        readDeque(s, coll, ctx);
    }

    template <class Stream, class T, class Alloc, class Context = FakeContext>
        requires sequential_output_stream<Stream>
    void write(Stream& s, const std::deque<T, Alloc>& coll, const Context& ctx = {})
    {
        writeCollection(s, coll, ctx);
    }

    template <class Stream, class T, class Compare, class Alloc, class Context = FakeContext>
        requires sequential_input_stream<Stream>
    void read(Stream & s, std::set<T, Compare, Alloc> & coll, const Context & ctx = {})
    {
        readCollection(s, coll, ctx);
    }

    template <class Stream, class T, class Compare, class Alloc, class Context = FakeContext>
        requires sequential_output_stream<Stream>
    void write(Stream & s, const std::set<T, Compare, Alloc> &coll, const Context & ctx = {})
    {
        writeCollection(s, coll, ctx);
    }

    template <class Stream, class T, class Compare, class Alloc, class Context = FakeContext>
        requires sequential_input_stream<Stream>
    void read(Stream & s, vector_set<T, Compare, Alloc> & coll, const Context & ctx = {})
    {
        readCollection(s, coll, ctx);
    }

    template <class Stream, class T, class Compare, class Alloc, class Context = FakeContext>
        requires sequential_output_stream<Stream>
    void write(Stream & s, const vector_set<T, Compare, Alloc> &coll, const Context & ctx = {})
    {
        writeCollection(s, coll, ctx);
    }

    template <class Stream, class T, class Compare, class Alloc, class Context = FakeContext>
        requires sequential_input_stream<Stream>
    void read(Stream & s, observable_vector_set<T, Compare, Alloc> & coll, const Context & ctx = {})
    {
        readCollection(s, coll, ctx);
    }

    template <class Stream, class T, class Compare, class Alloc, class Context = FakeContext>
        requires sequential_output_stream<Stream>
    void write(Stream & s, const observable_vector_set<T, Compare, Alloc> &coll, const Context & ctx = {})
    {
        writeCollection(s, coll, ctx);
    }

    template<class Stream, class T, class Hash, class KeyEqual, class Allocator, class Context = FakeContext>
        requires sequential_input_stream<Stream>
    void read(Stream & s, std::unordered_set<T, Hash, KeyEqual, Allocator> & coll, const Context & ctx = {})
    {
        readCollection(s, coll, ctx);
    }

    template<class Stream, class T, class Hash, class KeyEqual, class Allocator, class Context = FakeContext>
        requires sequential_output_stream<Stream>
    void write(Stream & s, const std::unordered_set<T, Hash, KeyEqual, Allocator> &coll, const Context & ctx = {})
    {
        writeCollection(s, coll, ctx);
    }

    template <class Stream, class Key, class T, class Compare, class Alloc, class Context = FakeContext>
        requires sequential_input_stream<Stream>
    void read(Stream & s, std::map<Key, T, Compare, Alloc> & coll, const Context & ctx = {})
    {
        readMap(s, coll, ctx);
    }

    template <class Stream, class Key, class T, class Compare, class Alloc, class Context = FakeContext>
        requires sequential_output_stream<Stream>
    void write(Stream & s, const std::map<Key, T, Compare, Alloc> &coll, const Context & ctx = {})
    {
        writeCollection(s, coll, ctx);
    }

    template<class Stream, class Key, class T, class Hash, class KeyEqual, class Allocator, class Context = FakeContext>
        requires sequential_input_stream<Stream>
    void read(Stream & s, std::unordered_map<Key, T, Hash, KeyEqual, Allocator> & coll, const Context & ctx = {})
    {
        readMap(s, coll, ctx);
    }

    template<class Stream, class T, class Key, class Hash, class KeyEqual, class Allocator, class Context = FakeContext>
        requires sequential_output_stream<Stream>
    void write(Stream & s, const std::unordered_map<Key, T, Hash, KeyEqual, Allocator> &coll, const Context & ctx = {})
    {
        writeCollection(s, coll, ctx);
    }

    // awl::ring should be initilized with a limit before it is read.
    template <class Stream, class T, class Alloc, class Context = FakeContext>
        requires sequential_input_stream<Stream>
    void read(Stream& s, awl::ring<T, Alloc>& coll, const Context& ctx = {})
    {
        readDeque(s, coll, ctx);
    }

    template <class Stream, class T, class Alloc, class Context = FakeContext>
        requires sequential_output_stream<Stream>
    void write(Stream& s, const awl::ring<T, Alloc>& coll, const Context& ctx = {})
    {
        writeCollection(s, coll, ctx);
    }
}
