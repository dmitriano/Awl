/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
#pragma once

#include "QtExtras/Json/JsonSerializer.h"
#include "QtExtras/Json/JsonException.h"

#include "Awl/StringFormat.h"

#include <charconv>
#include <chrono>
#include <format>
#include <sstream>
#include <string_view>

namespace awl
{
    template <class Clock, class Duration>
    class JsonSerializer<std::chrono::time_point<Clock, Duration>>
    {
    public:

        using value_type = std::chrono::time_point<Clock, Duration>;

        void FromJson(const QJsonValue & jv, value_type & v)
        {
            using namespace std::chrono;

            const double ms = jv.isString() ? std::stod(jv.toString().toStdString()) : jv.toDouble();
            const milliseconds::rep ms_count = static_cast<milliseconds::rep>(ms);
            v = value_type(milliseconds(ms_count));
        }

        void ToJson(const value_type & v, QJsonValue & jv)
        {
            using namespace std::chrono;
            jv = static_cast<double>(duration_cast<milliseconds>(v.time_since_epoch()).count());
        }
    };

    template <class Rep, class Period>
    class JsonSerializer<std::chrono::duration<Rep, Period>>
    {
    public:

        using value_type = std::chrono::duration<Rep, Period>;
        using formatted_type = std::chrono::hh_mm_ss<value_type>;
        using common_duration = std::chrono::nanoseconds;

        void FromJson(const QJsonValue& jv, value_type& v)
        {
            if (!jv.isString())
            {
                throw JsonException(awl::format() << "Expected duration as JSON string.");
            }

            QString text = jv.toString().trimmed();

            if (text.isEmpty())
            {
                throw JsonException(awl::format() << "Duration string is empty.");
            }

            const QString original_text = text;

            bool negative = false;

            if (text.startsWith('-'))
            {
                negative = true;
                text.remove(0, 1);
            }

            common_duration parsed{};

            if (!tryParseLegacyFormat(text, parsed))
            {
                parsed = parseExtendedFormat(text, original_text);
            }

            if (negative)
            {
                parsed = -parsed;
            }

            const value_type converted = std::chrono::duration_cast<value_type>(parsed);

            if (std::chrono::duration_cast<common_duration>(converted) != parsed)
            {
                throw JsonException(awl::format() << "Duration precision loss: " << original_text);
            }

            v = converted;
        }

        void ToJson(const value_type& v, QJsonValue& jv)
        {
            const common_duration common_value = std::chrono::duration_cast<common_duration>(v);
            const common_duration abs_value = common_value < common_duration::zero() ? -common_value : common_value;

            if (abs_value > std::chrono::duration_cast<common_duration>(std::chrono::days(1)))
            {
                jv = toExtendedFormat(common_value);
                return;
            }

            const formatted_type formatted(v);
            std::string text = std::format("{:%T}", formatted);

            if (text.contains('.'))
            {
                while (text.ends_with('0'))
                {
                    text.pop_back();
                }

                if (text.ends_with('.'))
                {
                    text.pop_back();
                }
            }

            jv = QString::fromStdString(text);
        }

    private:

        static bool tryParseLegacyFormat(const QString& text, common_duration& parsed)
        {
            std::istringstream in(text.toStdString());

            in >> std::chrono::parse("%T", parsed);

            if (in.fail())
            {
                return false;
            }

            in >> std::ws;

            return in.eof();
        }

        static QString toExtendedFormat(common_duration v)
        {
            bool negative = v < common_duration::zero();

            if (negative)
            {
                v = -v;
            }

            const auto day_duration = std::chrono::duration_cast<common_duration>(std::chrono::days(1));
            const auto hour_duration = std::chrono::duration_cast<common_duration>(std::chrono::hours(1));
            const auto minute_duration = std::chrono::duration_cast<common_duration>(std::chrono::minutes(1));
            const auto second_duration = std::chrono::duration_cast<common_duration>(std::chrono::seconds(1));

            const auto days = v / day_duration;
            v %= day_duration;

            const auto hours = v / hour_duration;
            v %= hour_duration;

            const auto minutes = v / minute_duration;
            v %= minute_duration;

            const auto seconds = v / second_duration;
            v %= second_duration;

            const auto fractional = v.count();

            std::string text;

            if (negative)
            {
                text += '-';
            }

            text += std::to_string(days);
            text += 'd';

            if (hours != 0)
            {
                text += '.';
                text += std::to_string(hours);
                text += 'h';
            }

            if (minutes != 0)
            {
                text += '.';
                text += std::to_string(minutes);
                text += 'm';
            }

            if (seconds != 0 || fractional != 0)
            {
                text += '.';
                text += std::to_string(seconds);
                text += 's';
            }

            if (fractional != 0)
            {
                std::string fraction = std::format("{:09}", fractional);

                while (fraction.ends_with('0'))
                {
                    fraction.pop_back();
                }

                text += '.';
                text += fraction;
            }

            return QString::fromStdString(text);
        }

        static common_duration parseExtendedFormat(const QString& text, const QString& original_text)
        {
            std::string storage = text.toStdString();
            std::string_view input(storage);
            common_duration total{};

            enum class Component
            {
                days,
                hours,
                minutes,
                seconds,
                fraction
            };

            Component last_component = Component::days;
            bool has_component = false;
            bool saw_seconds = false;
            bool saw_fraction = false;

            auto make_invalid_format = [&original_text]()
            {
                throw JsonException(awl::format() << "Invalid duration format: " << original_text);
            };

            auto parse_integer = [&original_text](std::string_view sv) -> int64_t
            {
                int64_t value{};
                const auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), value);

                if (ec != std::errc{} || ptr != sv.data() + sv.size())
                {
                    throw JsonException(awl::format() << "Invalid duration format: " << original_text);
                }

                return value;
            };

            while (!input.empty())
            {
                const size_t dot_pos = input.find('.');
                const std::string_view token = input.substr(0, dot_pos);

                if (token.empty())
                {
                    make_invalid_format();
                }

                if (token.back() == 'd' || token.back() == 'h' || token.back() == 'm' || token.back() == 's')
                {
                    if (saw_fraction)
                    {
                        make_invalid_format();
                    }

                    const std::string_view number = token.substr(0, token.size() - 1);

                    if (number.empty())
                    {
                        make_invalid_format();
                    }

                    const int64_t value = parse_integer(number);
                    Component component{};

                    switch (token.back())
                    {
                    case 'd':
                        component = Component::days;
                        total += std::chrono::duration_cast<common_duration>(std::chrono::days(value));
                        break;
                    case 'h':
                        component = Component::hours;
                        total += std::chrono::duration_cast<common_duration>(std::chrono::hours(value));
                        break;
                    case 'm':
                        component = Component::minutes;
                        total += std::chrono::duration_cast<common_duration>(std::chrono::minutes(value));
                        break;
                    case 's':
                        component = Component::seconds;
                        total += std::chrono::duration_cast<common_duration>(std::chrono::seconds(value));
                        saw_seconds = true;
                        break;
                    default:
                        make_invalid_format();
                    }

                    if (has_component && component <= last_component)
                    {
                        make_invalid_format();
                    }

                    has_component = true;
                    last_component = component;
                }
                else
                {
                    if (!saw_seconds || saw_fraction)
                    {
                        make_invalid_format();
                    }

                    saw_fraction = true;

                    int64_t fractional = 0;
                    const size_t max_digits = 9;
                    const size_t digits_to_parse = std::min(token.size(), max_digits);

                    if (digits_to_parse != 0)
                    {
                        fractional = parse_integer(token.substr(0, digits_to_parse));
                    }

                    for (size_t i = digits_to_parse; i < max_digits; ++i)
                    {
                        fractional *= 10;
                    }

                    for (size_t i = max_digits; i < token.size(); ++i)
                    {
                        if (token[i] != '0')
                        {
                            throw JsonException(awl::format() << "Duration precision loss: " << original_text);
                        }
                    }

                    total += common_duration(fractional);
                    last_component = Component::fraction;
                }

                if (dot_pos == std::string_view::npos)
                {
                    break;
                }

                input.remove_prefix(dot_pos + 1);
            }

            if (!has_component)
            {
                make_invalid_format();
            }

            return total;
        }
    };
}
