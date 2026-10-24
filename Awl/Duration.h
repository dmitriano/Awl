#pragma once

#include "Awl/Exception.h"

#include <algorithm>
#include <charconv>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <format>
#include <limits>
#include <ratio>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>

namespace awl
{
    namespace time_detail
    {
        template <class To, class Rep, class Period>
        To checked_duration_cast(const std::chrono::duration<Rep, Period> value)
        {
            using Target = typename To::rep;
            using Factor = std::ratio_divide<Period, typename To::period>;
            if constexpr (std::is_integral_v<Rep> && std::is_integral_v<Target>)
            {
                static_assert(sizeof(Rep) <= sizeof(std::uint64_t) && sizeof(Target) <= sizeof(std::uint64_t));
                const bool negative = value.count() < Rep{0};
                const std::uint64_t magnitude = negative
                    ? std::uint64_t{0} - static_cast<std::uint64_t>(value.count())
                    : static_cast<std::uint64_t>(value.count());
                if (magnitude % Factor::den != 0)
                {
                    throw GeneralException("Duration precision loss.");
                }

                std::uint64_t limit = static_cast<std::uint64_t>(std::numeric_limits<Target>::max());
                if (negative)
                {
                    if constexpr (std::is_signed_v<Target>)
                    {
                        ++limit;
                    }
                    else
                    {
                        throw GeneralException("Negative duration cannot be represented by an unsigned type.");
                    }
                }

                const std::uint64_t quotient = magnitude / Factor::den;
                if (quotient > limit / Factor::num)
                {
                    throw GeneralException("Duration is out of range.");
                }

                const std::uint64_t converted = quotient * Factor::num;
                if constexpr (std::is_signed_v<Target>)
                {
                    if (negative)
                    {
                        if (converted == limit)
                        {
                            return To(std::numeric_limits<Target>::lowest());
                        }

                        return To(-static_cast<Target>(converted));
                    }
                }

                return To(static_cast<Target>(converted));
            }
            else
            {
                const long double count = std::chrono::duration<long double, typename To::period>(value).count();
                if (!std::isfinite(count))
                {
                    throw GeneralException("Duration must be finite.");
                }

                if constexpr (std::is_integral_v<Target>)
                {
                    const long double upper = std::ldexp(1.0L, std::numeric_limits<Target>::digits);
                    const long double lower = std::is_signed_v<Target> ? -upper : 0.0L;
                    if (count < lower || count >= upper)
                    {
                        throw GeneralException("Duration is out of range.");
                    }

                    if (std::trunc(count) != count)
                    {
                        throw GeneralException("Duration precision loss.");
                    }
                }
                else if (count < std::numeric_limits<Target>::lowest() || count > std::numeric_limits<Target>::max())
                {
                    throw GeneralException("Duration is out of range.");
                }

                const Target converted = static_cast<Target>(count);
                if (static_cast<long double>(converted) != count)
                {
                    throw GeneralException("Duration precision loss.");
                }

                return To(converted);
            }
        }

        inline std::chrono::nanoseconds parse_duration_value(std::string_view input)
        {
            const auto invalid = []
            {
                throw GeneralException("Invalid duration format.");
            };
            if (input.empty())
            {
                invalid();
            }

            const bool negative = input.starts_with('-');
            if (negative)
            {
                input.remove_prefix(1);
            }

            const std::uint64_t limit = static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())
                + static_cast<std::uint64_t>(negative);
            std::uint64_t total = 0;
            int last_component = -1;
            bool saw_fraction = false;
            if (input.empty() || input.ends_with('.'))
            {
                invalid();
            }

            while (!input.empty())
            {
                const auto dot = input.find('.');
                const auto token = input.substr(0, dot);
                if (token.empty() || saw_fraction)
                {
                    invalid();
                }

                const auto number = [&invalid](const std::string_view digits)
                {
                    std::uint64_t result = 0;
                    const auto [end, error] = std::from_chars(digits.data(), digits.data() + digits.size(), result);
                    if (digits.empty() || error != std::errc{} || end != digits.data() + digits.size())
                    {
                        invalid();
                    }

                    return result;
                };
                int component = -1;
                std::uint64_t unit = 0;
                switch (token.back())
                {
                case 'd': component = 0; unit = 86400000000000; break;
                case 'h': component = 1; unit = 3600000000000; break;
                case 'm': component = 2; unit = 60000000000; break;
                case 's': component = 3; unit = 1000000000; break;
                }

                std::uint64_t amount = 0;
                if (component >= 0)
                {
                    if (component <= last_component)
                    {
                        invalid();
                    }

                    amount = number(token.substr(0, token.size() - 1));
                    last_component = component;
                }
                else if (token == "0" && last_component == -1)
                {
                    last_component = 3;
                    unit = 1;
                }
                else
                {
                    if (last_component != 3)
                    {
                        invalid();
                    }

                    const auto digits = std::min<std::size_t>(token.size(), 9);
                    amount = number(token.substr(0, digits));
                    for (auto index = digits; index < 9; ++index)
                    {
                        amount *= 10;
                    }

                    for (auto index = digits; index < token.size(); ++index)
                    {
                        if (token[index] != '0')
                        {
                            throw GeneralException("Duration precision loss.");
                        }
                    }

                    unit = 1;
                    saw_fraction = true;
                }

                if (amount > (limit - total) / unit)
                {
                    throw GeneralException("Duration is out of range.");
                }

                total += amount * unit;
                if (dot == std::string_view::npos)
                {
                    break;
                }

                input.remove_prefix(dot + 1);
            }

            if (negative && total == limit)
            {
                return std::chrono::nanoseconds(std::numeric_limits<std::int64_t>::lowest());
            }

            const auto count = static_cast<std::int64_t>(total);
            return std::chrono::nanoseconds(negative ? -count : count);
        }
    }

    constexpr uint8_t default_duration_part_count = 2;

    // Portable extended format with exact nanosecond precision. part_count is
    // retained for source compatibility; formatting never truncates components.
    template <class C, class Rep, class Period>
    void format_duration(std::basic_ostream<C>& out, const std::chrono::duration<Rep, Period> val,
        const uint8_t part_count = default_duration_part_count)
    {
        static_cast<void>(part_count);
        const auto ns = time_detail::checked_duration_cast<std::chrono::nanoseconds>(val);
        const bool negative = ns.count() < 0;
        std::uint64_t remaining = negative ? std::uint64_t{0} - static_cast<std::uint64_t>(ns.count())
            : static_cast<std::uint64_t>(ns.count());
        constexpr std::uint64_t day_duration = 86400000000000;
        constexpr std::uint64_t hour_duration = 3600000000000;
        constexpr std::uint64_t minute_duration = 60000000000;
        constexpr std::uint64_t second_duration = 1000000000;
        const auto days = remaining / day_duration;
        remaining %= day_duration;
        const auto hours = remaining / hour_duration;
        remaining %= hour_duration;
        const auto minutes = remaining / minute_duration;
        remaining %= minute_duration;
        const auto seconds = remaining / second_duration;
        remaining %= second_duration;
        const auto fractional = remaining;
        std::string text;
        bool has_whole_component = false;
        const auto append_component = [&text, &has_whole_component](const std::uint64_t value, const char suffix)
        {
            if (value != 0)
            {
                if (!text.empty() && text.back() != '-')
                {
                    text += '.';
                }

                text += std::format("{}{}", value, suffix);
                has_whole_component = true;
            }
        };
        if (negative)
        {
            text += '-';
        }

        append_component(days, 'd');
        append_component(hours, 'h');
        append_component(minutes, 'm');
        append_component(seconds, 's');
        if (fractional != 0)
        {
            if (!has_whole_component)
            {
                text += '0';
            }
            else if (seconds == 0)
            {
                text += ".0s";
            }

            std::string fraction = std::format("{:09}", fractional);
            while (fraction.ends_with('0'))
            {
                fraction.pop_back();
            }

            text += '.';
            text += fraction;
        }
        else if (!has_whole_component)
        {
            text += '0';
        }

        for (const char ch : text)
        {
            out.put(static_cast<C>(ch));
        }
    }

    template <class C, class Rep, class Period>
    std::basic_string<C> duration_to_string(const std::chrono::duration<Rep, Period> value,
        const uint8_t part_count = default_duration_part_count)
    {
        std::basic_ostringstream<C> out;
        format_duration(out, value, part_count);
        return out.str();
    }

    template <class Duration = std::chrono::nanoseconds>
    Duration parse_duration(const std::string_view text)
    {
        return time_detail::checked_duration_cast<Duration>(time_detail::parse_duration_value(text));
    }

    template <class Duration = std::chrono::nanoseconds>
    Duration parse_duration(const std::wstring_view text)
    {
        std::string narrow;
        narrow.reserve(text.size());
        for (const wchar_t ch : text)
        {
            if (ch < 0 || ch > 127)
            {
                throw GeneralException("Invalid duration format.");
            }

            narrow.push_back(static_cast<char>(ch));
        }

        return parse_duration<Duration>(narrow);
    }
}
