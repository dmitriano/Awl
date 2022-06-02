/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/DecimalData.h"
#include "Awl/String.h"

#include <cstdint>
#include <string>
#include <iostream>
#include <iomanip>
#include <cassert>
#include <limits>
#include <array>
#include <cmath>

namespace awl
{
    //The implementation of decimal class is not complete and I am not sure about its efficiency.
    //Consider using boost/multiprecision or decNumber Library, for example, or use std::decimal in GCC.
    
    //It does addition, subtraction and conversion to/from a string without loosing the precision.
    //Multiplication and division are performed via conversion to double.
    //An instance of decimal can be serialized as UInt with to_bits() and from_bits() methods,
    //but UInt value may be different with different compilers and platforms.

    template <typename UInt, uint8_t exp_len, template <typename, uint8_t> class DataTemplate = BuiltInDecimalData>
    class decimal
    {
    private:

        using Constants = helpers::DecimalConstants<UInt, exp_len>;
        using Data = DataTemplate<UInt, exp_len>;
        
        //static_assert(sizeof(Data) == sizeof(UInt));

    public:

        using Int = typename Data::Int;
        using Rep = typename Data::Rep;

        constexpr decimal()
        {
        }

        explicit constexpr decimal(uint8_t digits) : m_data(true, digits, 0)
        {
        }

        //We do not normalize it in the constructor.
        constexpr decimal(Int man, uint8_t digits) : decimal(digits)
        {
            set_mantissa(man);
        }

        template <class C>
        constexpr explicit decimal(std::basic_string_view<C> fixed_string)
        {
            *this = from_string(fixed_string);
        }

        template <class C>
        constexpr explicit decimal(const C* fixed_string) : decimal(std::basic_string_view<C>(fixed_string))
        {
        }

        decimal(const decimal& other) = default;
        decimal(decimal&& other) = default;

        decimal& operator = (const decimal& other) = default;
        decimal& operator = (decimal && other) = default;

        //The value of resulting UInt may be different with different compilers. But the value of Data structure
        //should be the same when I convert it back from UInt. See Bit field.
        static constexpr decimal from_bits(Rep val)
        {
            decimal a;
            a.m_data = Data::from_bits(val);
            return a;
        }

        constexpr Rep to_bits() const
        {
            return m_data.to_bits();
        }

        template <class Float>
        constexpr std::enable_if_t<std::is_arithmetic_v<Float>, Float> cast() const
        {
            return static_cast<Float>(static_cast<Float>(mantissa()) / static_cast<double>(denominator()));
        }

        //IEEE 754 double stores 2^53 without losing precision that is 15 decimal digits,
        //so the comparison with arithmetic (both integral and floating-point) types is performed
        //via conversion to double.
        constexpr operator double() const
        {
            return cast<double>();
        }

        constexpr bool positive() const
        {
            return m_data.positive();
        }

        constexpr bool negative() const
        {
            return !positive();
        }

        constexpr void negate()
        {
            m_data.set_positive(!m_data.positive());
        }

        constexpr Int mantissa() const
        {
            const Int signed_man = static_cast<Int>(m_data.man());

            return positive() ? signed_man : -signed_man;
        }

        constexpr void set_mantissa(Int val)
        {
            if (val >= 0)
            {
                m_data.set_positive(true);
                m_data.set_man(static_cast<UInt>(val));
            }
            else
            {
                m_data.set_positive(false);
                m_data.set_man(static_cast<UInt>(-val));
            }
        }
        
        constexpr UInt unsigned_mantissa() const
        {
            return m_data.man();
        }

        constexpr void set_unsigned_mantissa(UInt val)
        {
            m_data.set_man(val);
        }

        static constexpr UInt max_mantissa()
        {
            return Constants::max_man();
        }

        constexpr uint8_t exponent() const
        {
            return static_cast<uint8_t>(m_data.exp());
        }

        static constexpr uint8_t max_exponent()
        {
            return static_cast<uint8_t>(Constants::max_exp());
        }

        constexpr UInt denominator() const
        {
            return m_denoms[m_data.exp()];
        }

        template <class Float>
        constexpr std::enable_if_t<std::is_arithmetic_v<Float>, decimal&> operator = (Float val)
        {
            //We do not round the floating point value here so abs(decimal) < abs(val).
            set_mantissa(static_cast<Int>(val * static_cast<double>(denominator())));

            return *this;
        }

        bool operator == (const decimal& other) const
        {
            if (m_data.positive() == other.m_data.positive())
            {
                const decimal a = normalize();
                const decimal b = other.normalize();

                return a.m_data.exp() == b.m_data.exp() && a.m_data.man() == b.m_data.man();
            }
            
            return false;
        }

        bool operator != (const decimal& other) const
        {
            return !operator==(other);
        }

        bool operator < (const decimal& other) const
        {
            return compare(*this, other, std::less<UInt>());
        }

        bool operator > (const decimal& other) const
        {
            return compare(*this, other, std::greater<UInt>());
        }

        bool operator <= (const decimal& other) const
        {
            return compare(*this, other, std::less_equal<UInt>());
        }

        bool operator >= (const decimal& other) const
        {
            return compare(*this, other, std::greater_equal<UInt>());
        }

        constexpr decimal operator - () const
        {
            decimal temp = *this;

            temp.negate();

            return temp;
        }

        constexpr decimal operator + (const decimal& other) const
        {
            decimal a = *this;
            decimal b = other;

            align(a, b);

            return decimal(a.mantissa() + b.mantissa(), a.m_data.exp());
        }

        constexpr decimal operator - (const decimal& other) const
        {
            decimal a = *this;
            decimal b = other;

            align(a, b);

            return decimal(a.mantissa() - b.mantissa(), a.m_data.exp());
        }

        constexpr decimal& operator += (const decimal& other)
        {
            *this = *this + other;

            return *this;
        }

        constexpr decimal& operator -= (const decimal& other)
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
        static constexpr decimal from_string(std::basic_string_view<C> fixed_string);

        template <class C>
        constexpr std::basic_string<C> to_basic_string() const
        {
            std::basic_ostringstream<C> out;

            out << *this;

            return out.str();
        }

        std::basic_string<Char> to_string() const
        {
            return to_basic_string<Char>();
        }

        std::string to_astring() const
        {
            return to_basic_string<char>();
        }

        std::wstring to_wstring() const
        {
            return to_basic_string<wchar_t>();
        }

        constexpr decimal rescale(uint8_t digits, bool check = true) const
        {
            awl::decimal temp = *this;
            temp.rescale_self(digits, check);
            return temp;
        }

        constexpr decimal extend(uint8_t digits) const
        {
            awl::decimal temp = *this;
            temp.extend_self(digits);
            return temp;
        }

        constexpr decimal truncate(uint8_t digits) const
        {
            awl::decimal temp = *this;
            temp.trim_self(digits, false);
            return temp;
        }

        constexpr decimal normalize() const
        {
            awl::decimal temp = *this;
            temp.normalize_self();
            return temp;
        }

        constexpr Int rescaled_mantissa(uint8_t digits) const
        {
            return rescale(digits).mantissa();
        }

        static constexpr decimal zero()
        {
            return decimal();
        }

        template <class Float>
        static constexpr std::enable_if_t<std::is_arithmetic_v<Float>, decimal> make_decimal(Float val, uint8_t digits)
        {
            decimal d(digits);

            d = val;

            return d;
        }

        template <class Float>
        static constexpr std::enable_if_t<std::is_floating_point_v<Float>, decimal> make_rounded(Float val, uint8_t digits)
        {
            decimal d(digits);

            d.set_mantissa(static_cast<Int>(std::llround(val * static_cast<double>(d.denominator()))));

            return d;
        }

        template <class Float>
        static constexpr std::enable_if_t<std::is_integral_v<Float>, decimal> make_rounded(Float val, uint8_t digits)
        {
            return make_decimal(val, digits);
        }

        //Computes the smallest decimal value not less than arg.
        template <class Float>
        static constexpr std::enable_if_t<std::is_floating_point_v<Float>, decimal> make_ceiled(Float val, uint8_t digits)
        {
            decimal d(digits);

            d.set_mantissa(static_cast<Int>(std::ceil(val * static_cast<double>(d.denominator()))));

            return d;
        }

        template <class Float>
        static constexpr std::enable_if_t<std::is_integral_v<Float>, decimal> make_ceiled(Float val, uint8_t digits)
        {
            return make_decimal(val, digits);
        }

        //Computes the largest decimal value not greater than arg.
        template <class Float>
        static constexpr std::enable_if_t<std::is_floating_point_v<Float>, decimal> make_floored(Float val, uint8_t digits)
        {
            decimal d(digits);

            d.set_mantissa(static_cast<Int>(std::floor(val * static_cast<double>(d.denominator()))));

            return d;
        }

        template <class Float>
        static constexpr std::enable_if_t<std::is_integral_v<Float>, decimal> make_floored(Float val, uint8_t digits)
        {
            return make_decimal(val, digits);
        }

        //Computes the nearest decimal not greater in magnitude than arg.
        template <class Float>
        static constexpr std::enable_if_t<std::is_arithmetic_v<Float>, decimal> make_truncated(Float val, uint8_t digits)
        {
            return make_decimal(val, digits);
        }

    private:

        //If check==false losing precision (trimming) is allowed.
        constexpr void rescale_self(uint8_t digits, bool check = true)
        {
            if (m_data.exp() < digits)
            {
                extend_self(digits);
            }
            else if (digits < m_data.exp())
            {
                trim_self(digits, check);
            }
        }

        constexpr void extend_self(uint8_t digits)
        {
            if (m_data.exp() < digits)
            {
                check_exp(digits);

                //Nothing to do if the mantissa is zero.
                if (m_data.man() != 0)
                {
                    //the difference of two unsigned values is unsigned
                    const uint8_t diff = digits - exponent();

                    const UInt denom = m_denoms[diff];

                    const UInt max_diff = max_mantissa() / m_data.man();

                    if (denom > max_diff)
                    {
                        throw std::logic_error("Decimal overflow.");
                    }

                    m_data.set_man(m_data.man() * denom);
                }

                m_data.set_exp(digits);
            }
        }

        constexpr void trim_self(uint8_t digits, bool check)
        {
            if (digits < m_data.exp())
            {
                const uint8_t diff = exponent() - digits;

                const UInt denom = m_denoms[diff];

                if (check && m_data.man() % denom != 0)
                {
                    throw std::logic_error("Decimal is losing precision.");
                }

                m_data.set_man(m_data.man() / denom);

                m_data.set_exp(digits);
            }
        }

        //Removes traling zeros.
        constexpr void normalize_self()
        {
            while (m_data.man() != 0 && m_data.exp() != 0)
            {
                const Int remainder = m_data.man() % 10;

                if (remainder != 0)
                {
                    break;
                }

                m_data.set_man(m_data.man() / 10);
                m_data.set_exp(m_data.exp() - 1u);
            }

            if (m_data.man() == 0)
            {
                m_data.set_exp(0);
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

            //1 < 0

            auto make_sign = [](const Data& data) -> uint8_t { return data.positive() ? 0 : 1; };
            
            return comp(make_sign(b.m_data), make_sign(a.m_data));
        }

        template <class Comp>
        static constexpr bool compare_positive(const decimal& a, const decimal& b, Comp comp)
        {
            //We can't align decimals when we compare them.

            const UInt a_denom = a.denominator();
            const UInt b_denom = b.denominator();

            //Compare int parts.
            {
                const UInt a_val = a.m_data.man() / a_denom;
                const UInt b_val = b.m_data.man() / b_denom;

                if (a_val != b_val)
                {
                    return comp(a_val, b_val);
                }
            }

            //We can align fractional parts.
            {
                UInt a_val = a.m_data.man() % a_denom;
                UInt b_val = b.m_data.man() % b_denom;

                if (a.exponent() > b.exponent())
                {
                    const uint8_t diff = a.exponent() - b.exponent();

                    b_val *= m_denoms[diff];
                }
                else if (b.exponent() > a.exponent())
                {
                    const uint8_t diff = b.exponent() - a.exponent();

                    a_val *= m_denoms[diff];
                }

                return comp(a_val, b_val);
            }
        }

        template <class Float>
        constexpr std::enable_if_t<std::is_arithmetic_v<Float>, decimal> make_other(Float val) const
        {
            decimal other(exponent());

            other = val;

            return other;
        }

        template <class C>
        static constexpr std::tuple<typename std::basic_string_view<C>::const_iterator, uint8_t, UInt>
            parse_int(std::basic_string_view<C> fixed_string, bool point_terminator);

        template <class C>
        static constexpr uint8_t symbol_to_digit(C symbol)
        {
            if ('0' <= symbol && symbol <= '9')
            {
                return static_cast<uint8_t>(symbol - '0');
            }

            throw std::logic_error("Not a valid decimal string.");
        }

        static constexpr void check_exp(uint8_t digits)
        {
            if (digits > max_exponent())
            {
                throw std::logic_error("Too many digits in a decimal.");
            }
        }

        static constexpr void align(decimal& a, decimal& b)
        {
            if (a.m_data.exp() > b.m_data.exp())
            {
                b.rescale_self(a.m_data.exp());
            }
            else if (b.m_data.exp() > a.m_data.exp())
            {
                a.rescale_self(b.m_data.exp());
            }
        }

        static constexpr Constants::DenomArray m_denoms = Constants::make_denoms();
        
        Data m_data;
    };

    //Returns a normalized decimal, because we trimmed zeros.
    template <typename UInt, uint8_t exp_len, template <typename, uint8_t> class DataTemplate>
    template <class C>
    constexpr decimal<UInt, exp_len, DataTemplate> decimal<UInt, exp_len, DataTemplate>::from_string(std::basic_string_view<C> fixed_string)
    {
        using string_view = std::basic_string_view<C>;
        
        bool positive = true;

        {
            typename string_view::const_iterator i = fixed_string.begin();

            if (*i == '+' || *i == '-')
            {
                positive = *i == '+';

                ++i;
            }

            //Construction from the iterators is C++20 feature.
            fixed_string = string_view(i, fixed_string.end());
        }

        auto [i, int_digits, int_part] = parse_int(fixed_string, true);

        string_view fractional_text(i, fixed_string.end());

        auto [fractional_i, digits, fractional_part] = parse_int(fractional_text, false);

        if (fractional_i != fractional_text.end())
        {
            throw std::logic_error("Some characters left at the end of a decimal string.");
        }

        check_exp(digits);

        const UInt denom = m_denoms[digits];

        if (int_part > max_mantissa() / denom)
        {
            throw std::logic_error("Too long decimal string.");
        }
        
        int_part *= denom;

        if (fractional_part > max_mantissa() - int_part)
        {
            throw std::logic_error("Too long decimal string.");
        }

        decimal result;

        result.m_data.set_man(int_part + fractional_part);
        result.m_data.set_exp(digits);

        if (!positive)
        {
            result.negate();
        }

        return result;
    }

    template <typename UInt, uint8_t exp_len, template <typename, uint8_t> class DataTemplate, class C>
    std::basic_ostream<C>& operator << (std::basic_ostream<C>& out, const decimal<UInt, exp_len, DataTemplate> d)
    {
        if (d.negative())
        {
            out << '-';
        }

        const UInt man = d.unsigned_mantissa();

        const UInt denom = d.denominator();

        const UInt int_part = man / denom;

        const UInt fractional_part = man % denom;

        out << int_part;

        if (d.exponent() != 0)
        {
            out << "." << std::setfill(static_cast<C>('0')) << std::setw(d.exponent()) << fractional_part;
        }

        return out;
    }

    template <typename UInt, uint8_t exp_len, template <typename, uint8_t> class DataTemplate>
    template <class C>
    constexpr std::tuple<typename std::basic_string_view<C>::const_iterator, uint8_t, UInt>
        decimal<UInt, exp_len, DataTemplate>::parse_int(std::basic_string_view<C> fixed_string, bool point_terminator)
    {
        uint8_t digit_count = 0;

        uint8_t zero_count = 0;

        UInt val = 0;

        auto do_append = [&digit_count, &val](UInt digit)
        {
            val = val * 10 + digit;

            //It does not make a sense to parse a string longer than the mantissa.
            if (val > max_mantissa())
            {
                throw std::logic_error("A decimal string is too long.");
            }

            ++digit_count;
        };

        auto append = [&zero_count, &val, point_terminator, &do_append](UInt digit)
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

        auto i = fixed_string.begin();

        for (; i != fixed_string.end(); ++i)
        {
            C symbol = *i;

            if (point_terminator && symbol == '.')
            {
                i = i + 1;
                break;
            }

            const UInt digit = symbol_to_digit(symbol);

            append(digit);
        }

        return std::make_tuple(i, digit_count, val);
    }

    //Multiplies two decimals without loosing the precision and throws if an overflow occurs.
    template <typename UInt, uint8_t exp_len, template <typename, uint8_t> class DataTemplate>
    constexpr decimal<UInt, exp_len, DataTemplate> multiply(decimal<UInt, exp_len, DataTemplate> a, decimal<UInt, exp_len, DataTemplate> b)
    {
        decimal<UInt, exp_len, DataTemplate> c(a.exponent() + b.exponent());

        c.set_mantissa(a.mantissa() * b.mantissa());

        return c;
    }
}

namespace std
{
    template <typename UInt, uint8_t exp_len, template <typename, uint8_t> class DataTemplate>
    class numeric_limits<awl::decimal<UInt, exp_len, DataTemplate>>
    {
    private:

        using decimal = awl::decimal<UInt, exp_len, DataTemplate>;

    public:
        
        static constexpr decimal min() noexcept
        {
            return -decimal(decimal::max_mantissa(), 0);
        }

        static constexpr decimal max() noexcept
        {
            return decimal(decimal::max_mantissa(), 0);
        }
    };
}
