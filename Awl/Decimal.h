#pragma once

#include <cstdint>
#include <string>
#include <iostream>
#include <cassert>

#include "Awl/Exception.h"

namespace awl
{
    //IEEE 754 double stores 2^53 without losing precision.
    //Math.pow(2, 53) = 9007199254740992
    //Math.log10(Math.pow(2, 53)) = 15.954589770191003
    //so, it is 15 decimal digits.
    //The result of right - shifting a negative number in C++ is implementation - defined.

    //Do not use this class, if you worry about an extra 8 bytes used by the exponent :)
    //Consider using boost/multiprecision or decNumber Library, for example, or use std::decimal in GCC.
    class decimal
    {
    public:

        constexpr decimal(int64_t mantissa, uint8_t digits) : 
            m_man(mantissa), m_denom(calc_denom(digits))
        {
            check_digits(digits);
        }

        constexpr decimal(uint8_t digits) : decimal(0, digits)
        {
        }

        constexpr decimal() : decimal(0)
        {
        }

        template <class C>
        decimal(std::basic_string_view<C> text)
        {
            *this = from_string(text);
        }
            
        constexpr float to_float() const
        {
            return static_cast<float>(to_double());
        }

        constexpr double to_double() const
        {
            return static_cast<double>(m_man) / m_denom;
        }

        template <class C>
        static constexpr decimal from_string(std::basic_string_view<C> text);

        constexpr uint8_t digits() const
        {
            return static_cast<uint8_t>(calc_digits(m_denom));
        }

        constexpr void rescale(uint8_t digits)
        {
            check_digits(digits);
            
            const int64_t my_digits = calc_digits(m_denom);
            
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

    private:

        template <class C, class Int>
        static constexpr typename std::basic_string_view<C>::const_iterator parse_int(std::basic_string_view<C> text, bool point_terminator, Int& val)
        {
            val = 0;

            for (auto i = text.begin(); i != text.end(); ++i)
            {
                C symbol = *i;

                if (point_terminator && symbol == '.')
                {
                    return i + 1;
                }

                val = val * 10 + symbol_to_digit(symbol);
            }

            return text.end();
        }

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
            int64_t denom = 1;

            for (uint8_t i = 0; i < digits; ++i)
            {
                denom *= 10;
            }

            return denom;
        }

        static constexpr int64_t calc_digits(int64_t denom)
        {
            int64_t digits = 0;

            //1 means zero digits
            while (denom != 1)
            {
                denom /= 10;

                ++digits;
            }

            return digits;
        }

        //How many digits in the mantissa, "123.45" => 5, while denom is 100.
        static constexpr int64_t calc_man_digits(int64_t man)
        {
            int64_t digits = 0;

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

        int64_t m_man;
        int64_t m_denom;

        template <class C>
        friend std::basic_ostream<C>& operator << (std::basic_ostream<C>& out, const decimal d);
    };

    template <class C>
    static constexpr decimal decimal::from_string(std::basic_string_view<C> text)
    {
        int64_t int_part;

        auto i = parse_int(text, true, int_part);

        const auto fractional_i = i;

        //MSVC 2019 can't construct it from the iterators yet, it is C++20 feature.
        //std::basic_string_view<C> fractional_text(fractional_i, text.cend());
        std::basic_string_view<C> fractional_text(text.data() + (fractional_i - text.begin()), text.end() - fractional_i);

        int64_t fractional_part;

        i = parse_int(fractional_text, false, fractional_part);

        assert(i == fractional_text.end());

        const uint8_t digits = static_cast<uint8_t>(fractional_text.length());

        check_digits(digits);

        const int64_t denom = calc_denom(digits);

        int_part *= denom;

        return decimal(int_part + fractional_part, digits);
    }

    template <class C>
    std::basic_ostream<C>& operator << (std::basic_ostream<C>& out, const decimal d)
    {
        int64_t denom = d.calc_denom(decimal::maxDigits);

        int64_t man = d.m_man;

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
}
