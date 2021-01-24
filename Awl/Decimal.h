#pragma once

#include <cstdint>
#include <string>
#include <iostream>
#include <cassert>
#include <limits>

#include "Awl/Exception.h"

namespace awl
{
    //IEEE 754 double stores 2^53 without losing precision.
    //Math.pow(2, 53) = 9007199254740992
    //Math.log10(Math.pow(2, 53)) = 15.954589770191003
    //so, it is 15 decimal digits.
    //The result of right - shifting a negative number in C++ is implementation - defined.

    //Do not use this class, if you worry about an extra 8 bytes used by the exponent or something else :)
    //Consider using boost/multiprecision or decNumber Library, for example, or use std::decimal in GCC.
    class decimal
    {
    public:

        constexpr decimal() : m_denom(1), m_man(0)
        {
        }

        explicit constexpr decimal(uint8_t digits) :
            m_denom(calc_denom(digits)), m_man(0)
        {
        }

        //We do not normalize it in the constructor.
        constexpr decimal(int64_t mantissa, uint8_t digits) :
            m_denom(calc_denom(digits)), m_man(mantissa)
        {
        }

        template <class C>
        explicit decimal(std::basic_string_view<C> text)
        {
            *this = from_string(text);
        }
            
        template <class Float>
        constexpr std::enable_if_t<std::is_arithmetic_v<Float>, Float> cast() const
        {
            return static_cast<Float>(static_cast<Float>(m_man) / m_denom);
        }

        //The comparison with arithmetic types is performed via conversion to double.
        constexpr operator double() const
        {
            return cast<double>();
        }

        constexpr int64_t mantissa() const
        {
            return m_man;
        }
        
        constexpr uint8_t digits() const
        {
            return calc_digits(m_denom);
        }

        static constexpr uint8_t max_digits()
        {
            return maxDigits;
        }

        constexpr void rescale(uint8_t digits)
        {
            check_digits(digits);
            
            const int64_t my_digits = static_cast<int64_t>(calc_digits(m_denom));
            
            if (my_digits < digits)
            {
                //We add zeros.

                auto diff = digits - my_digits;

                check_digits(static_cast<uint8_t>(calc_man_digits(m_man) + diff));
                    
                for (auto i = 0; i < diff; ++i)
                {
                    m_man *= 10;
                    m_denom *= 10;
                }
            }
            else if (digits < my_digits)
            {
                //We loose some digits.

                auto diff = my_digits - digits;

                for (auto i = 0; i < diff; ++i)
                {
                    m_man /= 10;
                    m_denom /= 10;
                }
            }
        }

        //Removes traling zeros.
        constexpr void normalize()
        {
            while (m_man != 0 && m_denom != 1)
            {
                const int64_t remainder = m_man % 10;

                if (remainder != 0)
                {
                    break;
                }

                m_man /= 10;
                m_denom /= 10;
            }

            if (m_man == 0)
            {
                m_denom = 1;
            }
        }

        decimal& operator = (const decimal& other) = default;

        template <class Float>
        constexpr std::enable_if_t<std::is_arithmetic_v<Float>, decimal&> operator = (Float val)
        {
            //decimal temp(static_cast<int64_t>(val), 0);

            //temp.normalize();

            //check_digits(calc_man_digits(temp.mantissa()) + digits());
            
            m_man = static_cast<int64_t>(val * m_denom);

            return *this;
        }

        //auto operator <=> (const decimal& other) const
        //{
        //    return other.as_normalized_tie() <=> as_normalized_tie();
        //}

        bool operator == (const decimal& other) const
        {
            return other.as_normalized_tie() == as_normalized_tie();
        }

        bool operator != (const decimal& other) const
        {
            return !operator==(other);
        }

        bool operator < (const decimal& other) const
        {
            return as_normalized_tie() < other.as_normalized_tie();
        }

        bool operator > (const decimal& other) const
        {
            return as_normalized_tie() > other.as_normalized_tie();
        }

        bool operator <= (const decimal& other) const
        {
            return as_normalized_tie() <= other.as_normalized_tie();
        }

        bool operator >= (const decimal& other) const
        {
            return as_normalized_tie() >= other.as_normalized_tie();
        }

        decimal operator + (const decimal& other) const
        {
            decimal a = *this;
            decimal b = other;

            align(a, b);

            return decimal(a.m_man + b.m_man, calc_digits(a.m_denom));
        }

        decimal operator - (const decimal& other) const
        {
            decimal a = *this;
            decimal b = other;

            align(a, b);

            return decimal(a.m_man - b.m_man, calc_digits(a.m_denom));
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

        double operator * (const decimal& other) const
        {
            const double a = other.cast<double>();
            const double b = cast<double>();

            return a * b;
        }

        double operator / (const decimal& other) const
        {
            const double a = other.cast<double>();
            const double b = cast<double>();

            return b / a;
        }

        decimal& operator *= (const decimal& other)
        {
            *this = make_other(*this * other);

            return *this;
        }

        decimal& operator /= (const decimal& other)
        {
            *this = make_other(*this / other);

            return *this;
        }

        template <class C>
        static constexpr decimal from_string(std::basic_string_view<C> text);

    private:

        constexpr std::tuple<const int64_t&, const int64_t&> as_tie() const
        {
            return std::tie(m_denom, m_man);
        }

        constexpr std::tuple<int64_t&, int64_t&> as_tie()
        {
            return std::tie(m_denom, m_man);
        }

        constexpr awl::decimal as_normalized() const
        {
            awl::decimal temp = *this;
            temp.normalize();
            return temp;
        }

        constexpr std::tuple<int64_t, int64_t> as_normalized_tie() const
        {
            return as_normalized().as_tie();
        }

        template <class Float>
        constexpr std::enable_if_t<std::is_arithmetic_v<Float>, decimal> make_other(Float val)
        {
            decimal other(digits());

            other = val;

            return other;
        }

        template <class C>
        static constexpr std::tuple<typename std::basic_string_view<C>::const_iterator, uint8_t>
            parse_int(std::basic_string_view<C> text, bool point_terminator, int64_t& val);

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

        static constexpr int64_t calc_denom(uint8_t digits)
        {
            check_digits(digits);

            int64_t denom = 1;

            for (uint8_t i = 0; i < digits; ++i)
            {
                denom *= 10;
            }

            return denom;
        }

        static constexpr uint8_t calc_digits(int64_t denom)
        {
            uint8_t digits = 0;

            //1 means zero digits
            while (denom != 1)
            {
                denom /= 10;

                ++digits;
            }

            return digits;
        }

        //How many digits in the mantissa, "123.45" => 5, while denom is 100.
        static constexpr uint8_t calc_man_digits(int64_t man)
        {
            uint8_t digits = 0;

            while (man != 0)
            {
                man/= 10;

                ++digits;
            }

            return digits;
        }

        //Math.log10(Math.pow(2, 64)) = 19.265919722494797
        //Math.log10(Math.pow(2, 63)) = 18.964889726830815
        static constexpr uint8_t maxDigits = 18;

        static constexpr void check_digits(uint8_t digits)
        {
            if (digits > maxDigits)
            {
                throw std::runtime_error("Too many digits in a decimal.");
            }
        }

        static void align(decimal& a, decimal& b)
        {
            if (a.m_denom > b.m_denom)
            {
                b.rescale(calc_digits(a.m_denom));
            }
            else if (b.m_denom > a.m_denom)
            {
                a.rescale(calc_digits(b.m_denom));
            }
        }

        int64_t m_denom;
        int64_t m_man;

        template <class C>
        friend std::basic_ostream<C>& operator << (std::basic_ostream<C>& out, const decimal d);
    };

    template <class C>
    constexpr decimal decimal::from_string(std::basic_string_view<C> text)
    {
        int64_t int_part;

        auto [i, int_digits] = parse_int(text, true, int_part);

        //MSVC 2019 can't construct it from the iterators yet, it is C++20 feature.
        //std::basic_string_view<C> fractional_text(fractional_i, text.cend());
        std::basic_string_view<C> fractional_text(text.data() + (i - text.begin()), text.end() - i);

        int64_t fractional_part;

        auto [fractional_i, digits] = parse_int(fractional_text, false, fractional_part);

        assert(fractional_i == fractional_text.end());

        check_digits(int_digits + digits);

        const int64_t denom = calc_denom(digits);

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
        if (d.m_man < 0)
        {
            out << '-';
        }
        
        int64_t denom = d.calc_denom(decimal::maxDigits);

        int64_t man = std::abs(d.m_man);

        bool started = false;

        do
        {
            const int64_t digit = man / denom;

            if (digit != 0)
            {
                started = true;
            }

            if (started)
            {
                out << decimal::digit_to_symbol<C>(digit);
            }

            if (denom == d.m_denom)
            {
                if (!started)
                {
                    //leading zero before '.'
                    out << '0';
                }
                
                //Prevent 0 => "0.".
                if (denom != 1)
                {
                    out << '.';
                }

                started = true;
            }

            man %= denom;
            denom /= 10;
        }
        while (denom != 0);

        return out;
    }

    template <class C>
    constexpr std::tuple<typename std::basic_string_view<C>::const_iterator, uint8_t>
        decimal::parse_int(std::basic_string_view<C> text, bool point_terminator, int64_t& val)
    {
        uint8_t digit_count = 0;

        uint8_t zero_count = 0;

        val = 0;

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

        return std::make_tuple(i, digit_count);
    }

    template <class Float>
    constexpr std::enable_if_t<std::is_arithmetic_v<Float>, decimal> make_decimal(Float val, uint8_t digits)
    {
        decimal d(digits);

        d = val;

        return d;
    }
        
    inline constexpr awl::decimal zero;

    constexpr awl::decimal rescale(const awl::decimal& d, uint8_t digits)
    {
        awl::decimal temp = d;
        temp.rescale(digits);
        return temp;
    }

    constexpr awl::decimal normalize(const awl::decimal& d)
    {
        awl::decimal temp = d;
        temp.normalize();
        return temp;
    }
}

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
