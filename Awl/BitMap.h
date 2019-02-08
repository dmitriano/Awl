#pragma once

#include <type_traits>
#include <limits>
#include <bitset>
#include <initializer_list>

namespace awl
{
    template <class T> struct EnumTraits;

    template<typename Enum, bool IsEnum = std::is_enum<Enum>::value>
    class bitmap;

    template<typename Enum>
    class bitmap<Enum, true>
    {
    private:

        constexpr const static std::size_t N = EnumTraits<Enum>::count();
        typedef std::bitset<N> BitSet;

    public:

        constexpr bitmap() = default;

        constexpr bitmap(std::initializer_list<Enum> values)
        {
            for (Enum v : values)
            {
                m_bits.set(Enum2Index(v), true);
            }
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

        std::size_t count() const
        {
            return m_bits.count();
        }
        
        constexpr std::size_t size() const noexcept
        {
            return m_bits.size();
        }

        bool operator==(const bitmap& other) const
        {
            return m_bits == other.m_bits;
        }
        
        bool operator!=(const bitmap& other) const
        {
            return !operator == (other);
        }

        //It is not clear enough is it a good idea to have operator bool(), because, for example,
        //'bm1 == bm2' or 'bm1 == true' compiles even if operator == (…) is not defined.
        constexpr operator bool() const { return any(); }

    private:

        static constexpr std::size_t Enum2Index(Enum value)
        {
            return static_cast<std::size_t>(value);
        }

        static constexpr BitSet Enum2Bits(Enum value)
        {
            BitSet bits;
            bits.set(Enum2Index(value));
            return bits;
        }

        BitSet m_bits;
    };
}

#define AWL_BITMAP(EnumName, ...) \
    enum class EnumName { __VA_ARGS__ }; \
    template<> \
    struct awl::EnumTraits<EnumName> \
    { \
    private: \
        enum { __VA_ARGS__ }; \
    public: \
        static constexpr std::size_t count() \
        { \
            auto values = { __VA_ARGS__ }; \
            return static_cast<std::size_t>(values.end() - values.begin()); \
        } \
    };
