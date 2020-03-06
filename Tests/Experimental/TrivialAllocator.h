#pragma once

#include <memory>
#include <cstdint>
#include <cassert>

namespace awl
{
    class TrivialSpace
    {
    public:

        //TrivialSpace() = default;
        
        TrivialSpace(size_t size)
        {
            Reserve(size);
        }
        
        ~TrivialSpace()
        {
            Clear();
        }

        uint8_t * Allocate(std::size_t memory_size)
        {
            const size_t available_size = m_size - GetLength();
            assert(available_size >= memory_size);
            static_cast<void>(available_size);

            uint8_t * p = m_p;
            m_p += memory_size;
            return p;
        }

        void Reserve(size_t size)
        {
            Clear();
            m_size = size;
            pBuf = new uint8_t[size];
            Reset();
        }

        void Clear()
        {
            delete pBuf;
            m_size = 0;
            pBuf = nullptr;
            m_p = nullptr;
        }
        
        constexpr size_t GetCapacity() const
        {
            return m_size;
        }

        constexpr size_t GetLength() const
        {
            return static_cast<size_t>(m_p - pBuf);
        }

        constexpr bool IsFull() const
        {
            return GetLength() == GetCapacity();
        }

        void Reset()
        {
            std::fill(pBuf, pBuf + m_size, static_cast<uint8_t>(0u));
            m_p = pBuf;
        }

    private:

        size_t m_size = 0;
        uint8_t * pBuf = nullptr;
        uint8_t * m_p = nullptr;
    };
    
    template <class T>
    class TrivialAllocator
    {
    public:

        using value_type = T;

        TrivialAllocator(TrivialSpace & space) : m_space(space)
        {
        }

        template <class Q>
        TrivialAllocator(const TrivialAllocator<Q> & other) : m_space(other.m_space)
        {
        }

        T * allocate(std::size_t n)
        {
            return reinterpret_cast<T*>(m_space.Allocate(n * sizeof(T)));
        }

        void deallocate(T* p, std::size_t n)
        {
        }

    private:

        TrivialSpace & m_space;

        template <class Q>
        friend class TestAllocator;
    };
}
