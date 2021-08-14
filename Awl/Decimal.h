#pragma once

#include <cstdint>
#include <string>
#include <iostream>
#include <iomanip>
#include <cassert>
#include <limits>
#include <array>

#include "Awl/Exception.h"

namespace awl
{
    //AWL_DEFINE_EXCEPTION(DecimalException);
        
    namespace helpers
    {
        constexpr uint8_t sign_len = 1;
        constexpr uint8_t exp_len = 4;
        constexpr uint8_t man_len = 59;

        constexpr uint64_t p2(uint8_t n)
        {
            return static_cast<uint64_t>(1) << n;
        }

        constexpr uint64_t max_exp()
        {
            return p2(exp_len) - 1;
        }

        constexpr uint64_t max_man()
        {
            return p2(man_len) - 1;
        }

        constexpr uint8_t log10(uint8_t len)
        {
            uint64_t val = p2(len) - 1;
            
            uint8_t count = 0;

            while ((val /= 10) != 0)
            {
                ++count;
            }

            return count;
        }

        using DenomArray = std::array<uint64_t, max_exp()>;

        constexpr DenomArray make_denoms()
        {
            DenomArray a;

            uint64_t denom = 1;

            for (uint64_t e = 0; e < max_exp(); ++e)
            {
                a[e] = denom;

                denom *= 10;
            }

            return a;
        }
    }
    
    //Do not use this class, if you worry about its efficiency.
    //Consider using boost/multiprecision or decNumber Library, for example, or use std::decimal in GCC.
    class decimal
    {
    private:

        struct Data
        {
            //0 - positive, 1 - negative
            uint64_t sign : helpers::sign_len;
            uint64_t exp : helpers::exp_len;
            uint64_t man : helpers::man_len;
        };

        static_assert(sizeof(Data) == sizeof(uint64_t));

    public:

        constexpr decimal() : m_data{0, 0, 0}
        {
        }

        explicit constexpr decimal(uint8_t digits) : m_data{0, digits, 0}
        {
        }

        //We do not normalize it in the constructor.
        constexpr decimal(int64_t man, uint8_t digits) : decimal(digits)
        {
            set_mantissa(man);
        }

        template <class C>
        constexpr explicit decimal(std::basic_string_view<C> text)
        {
            *this = from_string(text);
        }

        static decimal from_int(uint64_t val)
        {
            decimal a;
            a.m_data = *(reinterpret_cast<const Data*>(&val));
            return a;
        }

        int64_t to_int() const
        {
            return *(reinterpret_cast<const int64_t*>(&m_data));
        }

        template <class Float>
        constexpr std::enable_if_t<std::is_arithmetic_v<Float>, Float> cast() const
        {
            return static_cast<Float>(static_cast<Float>(mantissa()) / denominator());
        }

        //The comparison with arithmetic types is performed via conversion to double.
        constexpr operator double() const
        {
            return cast<double>();
        }

        constexpr int64_t mantissa() const
        {
            const int64_t signed_man = static_cast<int64_t>(m_data.man);

            return positive() ? signed_man : -signed_man;
        }

        constexpr void set_mantissa(int64_t val)
        {
            if (val >= 0)
            {
                m_data.sign = 0;
                m_data.man = static_cast<int64_t>(val);
            }
            else
            {
                m_data.sign = 1;
                m_data.man = static_cast<int64_t>(-val);
            }
        }
        
        constexpr uint8_t exponent() const
        {
            return static_cast<uint8_t>(m_data.exp);
        }

        static constexpr uint8_t max_exponent()
        {
            return static_cast<uint8_t>(helpers::max_exp());
        }

        constexpr uint64_t denominator() const
        {
            return m_denoms[m_data.exp];
        }

        decimal& operator = (const decimal& other) = default;

        template <class Float>
        constexpr std::enable_if_t<std::is_arithmetic_v<Float>, decimal&> operator = (Float val)
        {
            set_mantissa(static_cast<int64_t>(val * denominator()));

            return *this;
        }

        bool operator == (const decimal& other) const
        {
            if (m_data.sign == other.m_data.sign)
            {
                const decimal a = normalize();
                const decimal b = other.normalize();

                return a.m_data.exp == b.m_data.exp && a.m_data.man == b.m_data.man;
            }
            
            return false;
        }

        bool operator != (const decimal& other) const
        {
            return !operator==(other);
        }

        bool operator < (const decimal& other) const
        {
            return compare(*this, other, std::less<int64_t>());
        }

        bool operator > (const decimal& other) const
        {
            return compare(*this, other, std::greater<int64_t>());
        }

        bool operator <= (const decimal& other) const
        {
            return compare(*this, other, std::less_equal<int64_t>());
        }

        bool operator >= (const decimal& other) const
        {
            return compare(*this, other, std::greater_equal<int64_t>());
        }

        decimal operator + (const decimal& other) const
        {
            decimal a = *this;
            decimal b = other;

            align(a, b);

            return decimal(a.mantissa() + b.mantissa(), a.m_data.exp);
        }

        decimal operator - (const decimal& other) const
        {
            decimal a = *this;
            decimal b = other;

            align(a, b);

            return decimal(a.mantissa() - b.mantissa(), a.m_data.exp);
        }

        decimal& operator += (const decimal& other)
        {
            *this = *this + other;

            return *this;
        }

        decimal& operator -= (const decimal& other)
        {
            *this = *this - other;

            return *this;
        }

        decimal operator + (double val) const
        {
            return *this + make_other(val);
        }

        decimal operator - (double val) const
        {
            return *this - make_other(val);
        }

        decimal& operator += (double val)
        {
            *this += make_other(val);

            return *this;
        }

        decimal& operator -= (double val)
        {
            *this -= make_other(val);

            return *this;
        }

        double operator * (double a) const
        {
            const double b = cast<double>();

            return a * b;
        }

        double operator / (double a) const
        {
            const double b = cast<double>();

            return b / a;
        }

        decimal& operator *= (double a)
        {
            *this = make_other(*this * a);

            return *this;
        }

        decimal& operator /= (double a)
        {
            *this = make_other(*this / a);

            return *this;
        }

        template <class C>
        static constexpr decimal from_string(std::basic_string_view<C> text);

        template <class C>
        constexpr std::basic_string<C> basic_to_string() const
        {
            std::basic_ostringstream<C> out;

            out << *this;

            return out.str();
        }

        std::basic_string<Char> to_string() const
        {
            return basic_to_string<Char>();
        }

        std::string to_astring() const
        {
            return basic_to_string<char>();
        }

        std::wstring to_wstring() const
        {
            return basic_to_string<wchar_t>();
        }

        constexpr decimal rescale(uint8_t digits) const
        {
            awl::decimal temp = *this;
            temp.rescale_self(digits);
            return temp;
        }

        constexpr decimal normalize() const
        {
            awl::decimal temp = *this;
            temp.normalize_self();
            return temp;
        }

        constexpr int64_t rescaled_mantissa(uint8_t digits) const
        {
            return rescale(digits).mantissa();
        }

    private:

        constexpr bool positive() const
        {
            return m_data.sign == 0;
        }

        constexpr bool negative() const
        {
            return !positive();
        }

        constexpr bool negate()
        {
            return m_data.sign = m_data.sign ? 0 : 1;
        }

        constexpr void rescale_self(uint8_t digits)
        {
            check_digits(digits);

            if (m_data.exp < digits)
            {
                //the difference of two unsigned values is unsigned
                const uint8_t diff = digits - exponent();

                const uint64_t max_diff = helpers::max_man() / m_data.man;

                const uint64_t denom = m_denoms[diff];
                
                if (denom > max_diff)
                {
                    throw std::logic_error("Decimal overflow.");
                }

                m_data.man *= denom;
            }
            else if (digits < m_data.exp)
            {
                const uint8_t diff = exponent() - digits;

                const uint64_t denom = m_denoms[diff];

                if (m_data.man % denom != 0)
                {
                    throw std::logic_error("Decimal underflow.");
                }
                
                m_data.man /= denom;
            }

            if (digits != m_data.exp)
            {
                m_data.exp = digits;
            }
        }

        //Removes traling zeros.
        constexpr void normalize_self()
        {
            while (m_data.man != 0 && m_data.exp != 0)
            {
                const int64_t remainder = m_data.man % 10;

                if (remainder != 0)
                {
                    break;
                }

                m_data.man /= 10;
                --m_data.exp;
            }

            if (m_data.man == 0)
            {
                m_data.exp = 0;
            }
        }

        template <class Comp>
        static constexpr bool compare(const decimal& a, const decimal& b, Comp comp)
        {
            if (a.negative() && b.negative())
            {
                return compare_positive(b, a, comp);
            }

            if (a.positive() && b.positive())
            {
                return compare_positive(a, b, comp);
            }

            return comp(a.m_data.sign, b.m_data.sign);
        }

        template <class Comp>
        static constexpr bool compare_positive(const decimal& a, const decimal& b, Comp comp)
        {
            //We can't align decimals when we compare them, but we can iterate over their digits.

            uint64_t a_denom = m_denoms[a.m_data.exp];
            uint64_t b_denom = m_denoms[b.m_data.exp];

            do
            {
                const uint64_t a_val = a.m_data.man / a_denom;
                const uint64_t b_val = b.m_data.man / b_denom;

                if (a_val != b_val)
                {
                    return comp(a_val, b_val);
                }

                a_denom /= 10;
                b_denom /= 10;
            }
            while (a_denom != 0 && b_denom != 0);

            return comp(a_denom, a_denom);
        }

        template <class Float>
        constexpr std::enable_if_t<std::is_arithmetic_v<Float>, decimal> make_other(Float val) const
        {
            decimal other(exponent());

            other = val;

            return other;
        }

        template <class C>
        static constexpr std::tuple<typename std::basic_string_view<C>::const_iterator, uint8_t, int64_t>
            parse_int(std::basic_string_view<C> text, bool point_terminator);

        template <class C>
        static constexpr int64_t symbol_to_digit(C symbol)
        {
            if ('0' <= symbol && symbol <= '9')
            {
                return static_cast<int64_t>(symbol - '0');
            }

            throw std::runtime_error("Not a valid decimal string.");
        }

        template <class C>
        static constexpr C digit_to_symbol(int64_t digit)
        {
            return static_cast<C>('0' + digit);
        }

        static constexpr void check_digits(uint8_t digits)
        {
            if (digits > max_exponent())
            {
                throw std::logic_error("Too many digits in a decimal.");
            }
        }

        static void align(decimal& a, decimal& b)
        {
            if (a.m_data.exp > b.m_data.exp)
            {
                b.rescale_self(a.m_data.exp);
            }
            else if (b.m_data.exp > a.m_data.exp)
            {
                a.rescale_self(b.m_data.exp);
            }
        }

        static constexpr helpers::DenomArray m_denoms = helpers::make_denoms();
        
        Data m_data;

        template <class C>
        friend std::basic_ostream<C>& operator << (std::basic_ostream<C>& out, const decimal d);
    };

    template <class C>
    constexpr decimal decimal::from_string(std::basic_string_view<C> text)
    {
        auto [i, int_digits, int_part] = parse_int(text, true);

        //MSVC 2019 can't construct it from the iterators yet, it is C++20 feature.
        //std::basic_string_view<C> fractional_text(fractional_i, text.cend());
        std::basic_string_view<C> fractional_text(text.data() + (i - text.begin()), text.end() - i);

        auto [fractional_i, digits, fractional_part] = parse_int(fractional_text, false);

        assert(fractional_i == fractional_text.end());

        check_digits(digits);

        const int64_t denom = m_denoms[digits];

        if (int_part < 0)
        {
            fractional_part = -fractional_part;
        }

        int_part *= denom;

        //It should be normalized, because we trimmed zeros.
        return decimal(int_part + fractional_part, digits);
    }

    template <class C>
    std::basic_ostream<C>& operator << (std::basic_ostream<C>& out, const decimal d)
    {
        if (d.negative())
        {
            out << '-';
        }

        const uint64_t man = d.m_data.man;

        const uint64_t denom = d.denominator();

        const uint64_t int_part = man / denom;

        const uint64_t fractional_part = man % denom;

        out << int_part;

        if (fractional_part != 0)
        {
            out << "." << std::setfill(static_cast<C>('0')) << std::setw(d.exponent()) << fractional_part;
        }

        return out;
    }

    template <class C>
    constexpr std::tuple<typename std::basic_string_view<C>::const_iterator, uint8_t, int64_t>
        decimal::parse_int(std::basic_string_view<C> text, bool point_terminator)
    {
        uint8_t digit_count = 0;

        uint8_t zero_count = 0;

        int64_t val = 0;

        auto do_append = [&digit_count, &val](int64_t digit)
        {
            val = val * 10 + digit;

            ++digit_count;
        };

        auto append = [&zero_count, &val, point_terminator, &do_append](int64_t digit)
        {
            if (point_terminator)
            {
                //Leading zeros are ignored when val is zero.
                if (digit != 0 || val != 0)
                {
                    do_append(digit);
                }
            }
            else
            {
                //Ignore trailing zeroz after decimal point.
                if (digit == 0)
                {
                    //accumulate leading zeros
                    ++zero_count;
                }
                else
                {
                    //flush accumulated zeros
                    for (uint8_t i = 0; i < zero_count; ++i)
                    {
                        do_append(0);
                    }

                    zero_count = 0;

                    do_append(digit);
                }
            }
        };

        auto i = text.begin();

        bool positive = true;

        if (i != text.end())
        {
            if (point_terminator)
            {
                if (*i == '+' || *i == '-')
                {
                    positive = *i == '+';

                    ++i;
                }
            }
            
            for (; i != text.end(); ++i)
            {
                C symbol = *i;

                if (point_terminator && symbol == '.')
                {
                    i = i + 1;
                    break;
                }

                const int64_t digit = symbol_to_digit(symbol);

                append(digit);
            }
        }

        if (!positive)
        {
            val = -val;
        }

        return std::make_tuple(i, digit_count, val);
    }

    template <class Float>
    constexpr std::enable_if_t<std::is_arithmetic_v<Float>, decimal> make_decimal(Float val, uint8_t digits)
    {
        decimal d(digits);

        d = val;

        return d;
    }
        
    inline constexpr awl::decimal zero;
}

/*
namespace std
{
    template <>
    class numeric_limits<awl::decimal>
    {
    public:
        
        static constexpr awl::decimal min() noexcept
        {
            return awl::decimal(numeric_limits<int64_t>::min(), 0);
        }

        static constexpr awl::decimal max() noexcept
        {
            return awl::decimal(numeric_limits<int64_t>::max(), 0);
        }
    };
}
*/
