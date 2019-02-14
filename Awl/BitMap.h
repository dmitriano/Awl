#pragma once

#include "EnumTraits.h"

#include <type_traits>
#include <bitset>
#include <initializer_list>

namespace awl
{
    template<typename Enum, typename std::underlying_type<Enum>::type N = EnumTraits<Enum>::count(), bool IsEnum = std::is_enum<Enum>::value>
    class bitmap;

    template<typename Enum, typename std::underlying_type<Enum>::type N>
    class bitmap<Enum, N, true>
    {
    private:

        typedef std::bitset<N> BitSet;

    public:

        typedef Enum enum_type;
        typedef typename std::underlying_type<enum_type>::type size_type;

        constexpr bitmap() = default;

        constexpr bitmap(std::initializer_list<Enum> values) : m_bits(MakeLong(values))
        {
            //for (Enum v : values)
            //{
            //    m_bits |= Enum2Bits(v);
            //}
        }

        constexpr bitmap operator|(Enum value) const
        {
            bitmap bm = *this;
            bm.m_bits |= Enum2Bits(value);
            return bm;
        }

        constexpr bitmap operator&(Enum value) const
        {
            bitmap bm = *this;
            bm.m_bits &= Enum2Bits(value);
            return bm;
        }

        constexpr bitmap operator^(Enum value) const
        {
            bitmap bm = *this;
            bm.m_bits ^= Enum2Bits(value);
            return bm;
        }

        constexpr bitmap operator~() const
        {
            bitmap bm = *this;
            bm.m_bits.flip();
            return bm;
        }

        constexpr bitmap& operator|=(Enum value)
        {
            m_bits |= Enum2Bits(value);
            return *this;
        }

        constexpr bitmap& operator&=(Enum value)
        {
            m_bits &= Enum2Bits(value);
            return *this;
        }

        constexpr bitmap& operator^=(Enum value)
        {
            m_bits ^= Enum2Bits(value);
            return *this;
        }

        constexpr bitmap operator|(const bitmap & other) const
        {
            bitmap bm = *this;
            bm.m_bits |= other.m_bits;
            return bm;
        }

        constexpr bitmap operator&(const bitmap & other) const
        {
            bitmap bm = *this;
            bm.m_bits &= other.m_bits;
            return bm;
        }

        constexpr bitmap operator^(const bitmap & other) const
        {
            bitmap bm = *this;
            bm.m_bits ^= other.m_bits;
            return bm;
        }

        constexpr bitmap& operator|=(const bitmap & other)
        {
            m_bits |= other.m_bits;
            return *this;
        }

        constexpr bitmap& operator&=(const bitmap & other)
        {
            m_bits &= other.m_bits;
            return *this;
        }

        constexpr bitmap& operator^=(const bitmap & other)
        {
            m_bits ^= other.m_bits;
            return *this;
        }

        constexpr bool test(Enum value) const
        {
            return m_bits.test(Enum2Index(value));
        }

        constexpr bool any() const { return m_bits.any(); }
        
        constexpr bool all() const { return m_bits.all(); }
        
        constexpr bool none() const { return m_bits.none(); }
        
        constexpr bitmap& set(Enum value, bool b)
        {
            m_bits.set(Enum2Index(value), b);

            return *this;
        }

        constexpr bool operator[](Enum value) const
        {
            return m_bits[Enum2Index(value)];
        }

        auto operator[](Enum value)
        {
            return m_bits[Enum2Index(value)];
        }

        size_type count() const
        {
            return static_cast<size_type>(m_bits.count());
        }
        
        constexpr size_type size() const noexcept
        {
            return static_cast<size_type>(m_bits.size());
        }

        bool operator==(const bitmap& other) const
        {
            return m_bits == other.m_bits;
        }
        
        bool operator!=(const bitmap& other) const
        {
            return !operator == (other);
        }

        constexpr operator bool() const { return any(); }

    private:

        static constexpr size_type Enum2Index(Enum value)
        {
            return static_cast<size_type>(value);
        }

        static constexpr BitSet Enum2Bits(Enum value)
        {
            BitSet bits;
            bits.set(Enum2Index(value));
            return bits;
        }

        static constexpr unsigned long long MakeLong(std::initializer_list<Enum> values)
        {
            unsigned long long val = 0ull;

            for (Enum v : values)
            {
                val |= (1ull << Enum2Index(v));
            }

            return val;
        }

        BitSet m_bits;
    };
}

#define AWL_BITMAP(EnumName, ...) \
    AWL_SEQUENTIAL_ENUM(EnumName, __VA_ARGS__) \
    typedef awl::bitmap<EnumName, EnumName##Traits::Count> EnumName##BitMap;
