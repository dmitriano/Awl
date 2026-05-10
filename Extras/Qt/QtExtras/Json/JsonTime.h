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
#include <string_view>
namespace awl
{
    template <class Clock, class Duration>
    class JsonSerializer<std::chrono::time_point<Clock, Duration>>
    {
    public:

        using value_type = std::chrono::time_point<Clock, Duration>;

        void fromJson(const QJsonValue & jv, value_type & v)
        {
            using namespace std::chrono;

            const double ms = jv.isString() ? std::stod(jv.toString().toStdString()) : jv.toDouble();
            const milliseconds::rep ms_count = static_cast<milliseconds::rep>(ms);
            v = value_type(milliseconds(ms_count));
        }

        void toJson(const value_type & v, QJsonValue & jv)
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
        using common_duration = std::chrono::nanoseconds;

        void fromJson(const QJsonValue& jv, value_type& v)
        {
            if (!jv.isString())
            {
                throw JsonException(_T("Expected duration as JSON string."));
            }

            QString text = jv.toString().trimmed();

            if (text.isEmpty())
            {
                throw JsonException(_T("Duration string is empty."));
            }

            const QString original_text = text;

            bool negative = false;

            if (text.startsWith('-'))
            {
                negative = true;
                text.remove(0, 1);
            }

            common_duration parsed{};

            try
            {
                parsed = parseExtendedFormat(text, original_text);
            }
            catch (const JsonException& e)
            {
                throwWrongDurationValue(original_text, e.message());
            }

            if (negative)
            {
                parsed = -parsed;
            }

            const value_type converted = std::chrono::duration_cast<value_type>(parsed);

            if (std::chrono::duration_cast<common_duration>(converted) != parsed)
            {
                throwWrongDurationValue(original_text, _T("Duration precision loss."));
            }

            v = converted;
        }

        void toJson(const value_type& v, QJsonValue& jv)
        {
            const common_duration common_value = std::chrono::duration_cast<common_duration>(v);
            jv = toExtendedFormat(common_value);
        }

    private:

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
            bool has_whole_component = false;

            auto append_component = [&text, &has_whole_component](int64_t value, char suffix)
            {
                if (value == 0)
                {
                    return;
                }

                if (!text.empty() && text.back() != '-')
                {
                    text += '.';
                }

                text += std::to_string(value);
                text += suffix;
                has_whole_component = true;
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
                    if (!text.empty() && text.back() != '-')
                    {
                        text += '.';
                    }

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
                throw JsonException(std::format(_T("Invalid duration format: {}"), original_text));
            };

            auto parse_integer = [&original_text](std::string_view sv) -> int64_t
            {
                int64_t value{};
                const auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), value);

                if (ec != std::errc{} || ptr != sv.data() + sv.size())
                {
                    throw JsonException(std::format(_T("Invalid duration format: {}"), original_text));
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
                else if (!saw_seconds && !saw_fraction && token == "0")
                {
                    if (has_component && Component::seconds <= last_component)
                    {
                        make_invalid_format();
                    }

                    saw_seconds = true;
                    has_component = true;
                    last_component = Component::seconds;
                }
                else
                {
                    if (saw_fraction)
                    {
                        make_invalid_format();
                    }

                    saw_seconds = true;
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
                            throw JsonException(std::format(_T("Duration precision loss: {}"), original_text));
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

        static void throwWrongDurationValue(const QString& text, const awl::String& details)
        {
            throw JsonException(std::format(_T("Wrong duration value {}. {}"), text, details));
        }
    };
}
