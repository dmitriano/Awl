#pragma once

#include "Awl/EnumTraits.h"
#include "Awl/Reflection.h"
#include "BoostExtras/Json/Json.h"

#include <boost/json.hpp>
#include <boost/locale/encoding_utf.hpp>
#include <atomic>
#include <chrono>
#include <functional>
#include <map>
#include <cmath>
#include <concepts>
#include <cstdint>
#include <format>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <typeindex>
#include <utility>
#include <vector>

namespace awl
{
    // A custom serializer cannot reveal its wire shape. Supply schema() and
    // optionally validate(value); otherwise validation uses its fromJson().
    template <class T>
    struct JsonSchemaTraits {};

    namespace json_schema_detail
    {
        template <class T>
        struct OptionalTraits : std::false_type {};

        template <class T>
        struct OptionalTraits<std::optional<T>> : std::true_type
        {
            using Value = T;
        };

        template <class T>
        struct WrapperTraits : std::false_type {};

        template <class T>
        struct WrapperTraits<std::atomic<T>> : std::true_type { using Value = T; };

        template <class T>
        struct WrapperTraits<std::reference_wrapper<T>> : std::true_type { using Value = std::remove_cv_t<T>; };

        template <class T>
        struct TupleTraits : std::false_type {};

        template <class... Args>
        struct TupleTraits<std::tuple<Args...>> : std::true_type {};

        template <class T>
        struct DurationTraits : std::false_type {};

        template <class Rep, class Period>
        struct DurationTraits<std::chrono::duration<Rep, Period>> : std::true_type {};

        template <class T>
        struct TimePointTraits : std::false_type {};

        template <class Clock, class Duration>
        struct TimePointTraits<std::chrono::time_point<Clock, Duration>> : std::true_type {};

        template <class T>
        struct DecimalTraits : std::false_type {};

        template <class UInt, std::uint8_t Exponent, template <class, std::uint8_t> class Data>
        struct DecimalTraits<awl::decimal<UInt, Exponent, Data>> : std::true_type {};

        template <class T>
        concept StringMap = awl::insertable_map<T> && std::same_as<typename T::key_type, std::string>;

        template <class T>
        concept JsonSerializable = requires(awl::JsonSerializer<T> serializer, T& output,
            const T& input, boost::json::value& json)
        {
            serializer.fromJson(json, output);
            serializer.toJson(input, json);
        };

        template <class T>
        constexpr bool isOptional()
        {
            if constexpr (WrapperTraits<T>::value)
            {
                return isOptional<typename WrapperTraits<T>::Value>();
            }
            else
            {
                return OptionalTraits<T>::value;
            }
        }

        struct SchemaContext
        {
            std::map<std::type_index, std::string> references;
            boost::json::object definitions;

            template <class T>
            boost::json::object reference()
            {
                const auto [item, inserted] = references.try_emplace(typeid(T),
                    std::format("type{}", references.size()));
                return {{"$ref", std::format("#/$defs/{}", item->second)}};
            }

            template <class T>
            void save(const boost::json::object& schema)
            {
                if (const auto item = references.find(typeid(T)); item != references.end())
                {
                    definitions[item->second] = schema;
                }
            }
        };

        template <class T>
        inline constexpr bool unsupportedJsonType = false;

        template <awl::reflectable T, class Func>
        void forEachField(Func&& function)
        {
            using Fields = decltype(std::declval<T&>().as_tuple());
            [&]<std::size_t... Indices>(std::index_sequence<Indices...>)
            {
                (function.template operator()<std::remove_cvref_t<std::tuple_element_t<Indices, Fields>>>(
                    T::member_names()[Indices]), ...);
            }(std::make_index_sequence<std::tuple_size_v<Fields>>{});
        }

        template <std::floating_point T>
        constexpr double floatingLimit()
        {
            return static_cast<double>(std::min<long double>(std::numeric_limits<T>::max(),
                std::numeric_limits<double>::max()));
        }

        template <class T, class... Ancestors>
        boost::json::object buildJsonSchema(SchemaContext& context);

        template <class T, class... Ancestors>
        boost::json::object buildJsonSchemaBody(SchemaContext& context)
        {
            namespace json = boost::json;

            if constexpr (requires { JsonSchemaTraits<T>::schema(); })
            {
                return JsonSchemaTraits<T>::schema();
            }
            else if constexpr (WrapperTraits<T>::value)
            {
                return buildJsonSchema<typename WrapperTraits<T>::Value, Ancestors..., T>(context);
            }
            else if constexpr (OptionalTraits<T>::value)
            {
                return json::object{{"anyOf", json::array{
                    buildJsonSchema<typename OptionalTraits<T>::Value, Ancestors..., T>(context),
                    json::object{{"type", "null"}}}}};
            }
            else if constexpr (StringMap<T>)
            {
                return {{"type", "object"}, {"additionalProperties",
                    buildJsonSchema<typename T::mapped_type, Ancestors..., T>(context)}};
            }
            else if constexpr (awl::inserter_defined<T>)
            {
                return {{"type", "array"}, {"items",
                    buildJsonSchema<typename T::value_type, Ancestors..., T>(context)}};
            }
            else if constexpr (TupleTraits<T>::value)
            {
                json::array items;
                [&]<std::size_t... Indices>(std::index_sequence<Indices...>)
                {
                    (items.push_back(buildJsonSchema<std::remove_cvref_t<std::tuple_element_t<Indices, T>>,
                        Ancestors..., T>(context)), ...);
                }(std::make_index_sequence<std::tuple_size_v<T>>{});
                json::object schema{{"type", "array"}, {"minItems", std::tuple_size_v<T>},
                    {"maxItems", std::tuple_size_v<T>}, {"items", false}};

                if (!items.empty())
                {
                    schema["prefixItems"] = std::move(items);
                }

                return schema;
            }
            else if constexpr (TimePointTraits<T>::value)
            {
                json::object schema = buildJsonSchema<std::int64_t>(context);
                schema["description"] = "Milliseconds since the clock epoch";
                return schema;
            }
            else if constexpr (DurationTraits<T>::value || DecimalTraits<T>::value)
            {
                return {{"type", "string"}, {"description", DurationTraits<T>::value
                    ? "AWL duration, for example 1s.5 or -0.001" : "Exact AWL decimal string"}};
            }
            else if constexpr (std::same_as<T, json::value>)
            {
                return {};
            }
            else if constexpr (std::same_as<T, json::object>)
            {
                return {{"type", "object"}};
            }
            else if constexpr (std::same_as<T, json::array>)
            {
                return {{"type", "array"}};
            }
            else if constexpr (std::is_same_v<T, bool>)
            {
                return json::object{{"type", "boolean"}};
            }
            else if constexpr (std::is_integral_v<T>)
            {
                static_assert(sizeof(T) <= sizeof(std::uint64_t), "JSON integers support at most 64 bits.");

                if constexpr (std::is_signed_v<T>)
                {
                    return json::object{{"type", "integer"},
                        {"minimum", static_cast<std::int64_t>(std::numeric_limits<T>::lowest())},
                        {"maximum", static_cast<std::int64_t>(std::numeric_limits<T>::max())}};
                }
                else
                {
                    return json::object{{"type", "integer"}, {"minimum", 0},
                        {"maximum", static_cast<std::uint64_t>(std::numeric_limits<T>::max())}};
                }
            }
            else if constexpr (std::is_floating_point_v<T>)
            {
                const double limit = floatingLimit<T>();
                return json::object{{"type", "number"}, {"minimum", -limit}, {"maximum", limit}};
            }
            else if constexpr (std::same_as<T, std::string> || std::same_as<T, std::wstring>)
            {
                return json::object{{"type", "string"}};
            }
            else if constexpr (awl::sequential_enum<T>)
            {
                json::array names;

                for (const std::string& name : awl::EnumTraits<T>::names())
                {
                    names.push_back(json::value(name));
                }

                return json::object{{"type", "string"}, {"enum", std::move(names)}};
            }
            else if constexpr (awl::reflectable<T>)
            {
                json::object properties;
                json::array required;
                forEachField<T>([&]<class Field>(const std::string& name)
                {
                    properties[name] = buildJsonSchema<Field, Ancestors..., T>(context);

                    if constexpr (!isOptional<Field>())
                    {
                        required.push_back(json::value(name));
                    }
                });
                return json::object{{"type", "object"}, {"properties", std::move(properties)},
                    {"required", std::move(required)}, {"additionalProperties", false}};
            }
            else if constexpr (JsonSerializable<T>)
            {
                // The serializer alone does not describe its accepted JSON shape.
                return {};
            }
            else
            {
                static_assert(unsupportedJsonType<T>, "No AWL JsonSerializer for this C++ type.");
            }
        }

        template <class T, class... Ancestors>
        boost::json::object buildJsonSchema(SchemaContext& context)
        {
            if constexpr ((std::same_as<T, Ancestors> || ...))
            {
                return context.reference<T>();
            }
            else
            {
                boost::json::object schema = buildJsonSchemaBody<T, Ancestors...>(context);
                context.save<T>(schema);
                return schema;
            }
        }

        [[noreturn]] inline void invalidJson(const std::string& path, const std::string_view reason)
        {
            throw awl::JsonException(std::format("{}: {}", path, reason));
        }

        template <std::integral T>
        bool integerFits(const boost::json::value& value)
        {
            if (value.is_double())
            {
                const double number = value.as_double();
                const double upper = std::ldexp(1.0, std::numeric_limits<T>::digits);
                const double lower = std::is_signed_v<T> ? -upper : 0.0;
                return std::isfinite(number) && std::trunc(number) == number && number >= lower && number < upper;
            }

            if constexpr (std::is_signed_v<T>)
            {
                if (value.is_int64())
                {
                    return value.as_int64() >= static_cast<std::int64_t>(std::numeric_limits<T>::lowest())
                        && value.as_int64() <= static_cast<std::int64_t>(std::numeric_limits<T>::max());
                }

                return value.is_uint64()
                    && value.as_uint64() <= static_cast<std::uint64_t>(std::numeric_limits<T>::max());
            }
            else
            {
                if (value.is_int64())
                {
                    return value.as_int64() >= 0
                        && static_cast<std::uint64_t>(value.as_int64()) <= static_cast<std::uint64_t>(std::numeric_limits<T>::max());
                }

                return value.is_uint64()
                    && value.as_uint64() <= static_cast<std::uint64_t>(std::numeric_limits<T>::max());
            }
        }

        template <class T>
        void validateSerializedJson(const boost::json::value& value, const std::string& path)
        {
            static_assert(std::default_initializable<T>,
                "Supply JsonSchemaTraits<T>::validate for non-default-constructible custom types.");
            try
            {
                T parsed{};
                awl::JsonSerializer<T>{}.fromJson(value, parsed);
            }
            catch (const awl::JsonException& error)
            {
                invalidJson(path, error.what());
            }
        }

        template <class T>
        void validateJsonValue(const boost::json::value& value, const std::string& path)
        {
            if constexpr (requires { JsonSchemaTraits<T>::validate(value); })
            {
                try
                {
                    JsonSchemaTraits<T>::validate(value);
                }
                catch (const awl::JsonException& error)
                {
                    invalidJson(path, error.what());
                }
            }
            else if constexpr (requires { JsonSchemaTraits<T>::schema(); })
            {
                validateSerializedJson<T>(value, path);
            }
            else if constexpr (WrapperTraits<T>::value)
            {
                validateJsonValue<typename WrapperTraits<T>::Value>(value, path);
            }
            else if constexpr (OptionalTraits<T>::value)
            {
                if (!value.is_null())
                {
                    validateJsonValue<typename OptionalTraits<T>::Value>(value, path);
                }
            }
            else if constexpr (awl::inserter_defined<T>)
            {
                if (!value.is_array())
                {
                    invalidJson(path, "Expected an array");
                }

                const auto& items = value.as_array();

                for (std::size_t index = 0; index < items.size(); ++index)
                {
                    validateJsonValue<typename T::value_type>(items[index], std::format("{}[{}]", path, index));
                }
            }
            else if constexpr (StringMap<T>)
            {
                if (!value.is_object())
                {
                    invalidJson(path, "Expected an object");
                }

                for (const auto& item : value.as_object())
                {
                    validateJsonValue<typename T::mapped_type>(item.value(), std::format("{}.{}", path, item.key()));
                }
            }
            else if constexpr (TupleTraits<T>::value)
            {
                if (!value.is_array() || value.as_array().size() != std::tuple_size_v<T>)
                {
                    invalidJson(path, "Expected a tuple array of exactly the declared size");
                }

                [&]<std::size_t... Indices>(std::index_sequence<Indices...>)
                {
                    (validateJsonValue<std::remove_cvref_t<std::tuple_element_t<Indices, T>>>(
                        value.as_array()[Indices], std::format("{}[{}]", path, Indices)), ...);
                }(std::make_index_sequence<std::tuple_size_v<T>>{});
            }
            else if constexpr (std::same_as<T, boost::json::value>)
            {
            }
            else if constexpr (std::same_as<T, boost::json::object> || std::same_as<T, boost::json::array>)
            {
                if (!(std::same_as<T, boost::json::object> ? value.is_object() : value.is_array()))
                {
                    invalidJson(path, "Incorrect JSON DOM shape");
                }
            }
            else if constexpr (DurationTraits<T>::value || DecimalTraits<T>::value || TimePointTraits<T>::value)
            {
                if constexpr (TimePointTraits<T>::value)
                {
                    if (!integerFits<std::int64_t>(value))
                    {
                        invalidJson(path, "Expected epoch milliseconds as a signed 64-bit integer");
                    }
                }
                else if (!value.is_string())
                {
                    invalidJson(path, "Expected a string");
                }

                validateSerializedJson<T>(value, path);
            }
            else if constexpr (std::is_same_v<T, bool>)
            {
                if (!value.is_bool())
                {
                    invalidJson(path, "Expected a boolean");
                }
            }
            else if constexpr (std::is_integral_v<T>)
            {
                static_assert(sizeof(T) <= sizeof(std::uint64_t), "JSON integers support at most 64 bits.");

                if (!integerFits<T>(value))
                {
                    invalidJson(path, "Expected an integer within the C++ type range");
                }
            }
            else if constexpr (std::is_floating_point_v<T>)
            {
                if (!value.is_number())
                {
                    invalidJson(path, "Expected a number");
                }

                const double number = value.is_double() ? value.as_double()
                    : value.is_int64() ? static_cast<double>(value.as_int64()) : static_cast<double>(value.as_uint64());

                if (!std::isfinite(number) || number < -floatingLimit<T>() || number > floatingLimit<T>())
                {
                    invalidJson(path, "Number is outside the C++ type range");
                }
            }
            else if constexpr (std::same_as<T, std::string> || std::same_as<T, std::wstring>)
            {
                if (!value.is_string())
                {
                    invalidJson(path, "Expected a string");
                }
            }
            else if constexpr (awl::sequential_enum<T>)
            {
                if (!value.is_string())
                {
                    invalidJson(path, "Expected an enum name");
                }

                const auto& names = awl::EnumTraits<T>::names();

                if (names.find(std::string(value.as_string())) == names.end())
                {
                    invalidJson(path, "Unknown enum name");
                }
            }
            else if constexpr (awl::reflectable<T>)
            {
                if (!value.is_object())
                {
                    invalidJson(path, "Expected an object");
                }

                const auto& object = value.as_object();
                const auto& names = T::member_names();

                for (const auto& member : object)
                {
                    const std::string name(member.key());

                    if (names.find(name) == names.end())
                    {
                        invalidJson(std::format("{}.{}", path, name), "Unknown field");
                    }
                }

                forEachField<T>([&]<class Field>(const std::string& name)
                {
                    const std::string field_path = std::format("{}.{}", path, name);

                    if (const boost::json::value* field = object.if_contains(name))
                    {
                        validateJsonValue<Field>(*field, field_path);
                    }
                    else if constexpr (!isOptional<Field>())
                    {
                        invalidJson(field_path, "Required field is missing");
                    }
                });
            }
            else if constexpr (JsonSerializable<T>)
            {
                validateSerializedJson<T>(value, path);
            }
            else
            {
                static_assert(unsupportedJsonType<T>, "No AWL JsonSerializer for this C++ type.");
            }
        }

        // Validate raw DOM payloads as well as typed fields before serialization.
        inline void validateJsonDocument(const boost::json::value& value, const std::string& path)
        {
            const auto validate_text = [&path](const std::string_view text)
            {
                try
                {
                    boost::locale::conv::utf_to_utf<char32_t>(text.data(), text.data() + text.size(), boost::locale::conv::stop);
                }
                catch (const boost::locale::conv::conversion_error&)
                {
                    invalidJson(path, "Invalid UTF-8 string");
                }
            };

            if (value.is_double() && !std::isfinite(value.as_double()))
            {
                invalidJson(path, "JSON numbers must be finite");
            }
            else if (value.is_string())
            {
                validate_text(value.as_string());
            }
            else if (value.is_array())
            {
                for (std::size_t index = 0; index < value.as_array().size(); ++index)
                {
                    validateJsonDocument(value.as_array()[index], std::format("{}[{}]", path, index));
                }
            }
            else if (value.is_object())
            {
                for (const auto& item : value.as_object())
                {
                    validate_text(item.key());
                    validateJsonDocument(item.value(), std::format("{}.{}", path, item.key()));
                }
            }
        }

        // Normalize only typed integers. Opaque JSON and floating point fields
        // retain their original kinds; uint64_t never passes through int64_t.
        template <class T>
        void normalizeJsonIntegers(boost::json::value& value)
        {
            if constexpr (requires { JsonSchemaTraits<T>::schema(); })
            {
            }
            else if constexpr (WrapperTraits<T>::value)
            {
                normalizeJsonIntegers<typename WrapperTraits<T>::Value>(value);
            }
            else if constexpr (OptionalTraits<T>::value)
            {
                if (!value.is_null())
                {
                    normalizeJsonIntegers<typename OptionalTraits<T>::Value>(value);
                }
            }
            else if constexpr (std::integral<T> && !std::same_as<T, bool>)
            {
                if (value.is_double())
                {
                    value = value.as_double() >= 0 ? boost::json::value(static_cast<std::uint64_t>(value.as_double()))
                        : boost::json::value(static_cast<std::int64_t>(value.as_double()));
                }
            }
            else if constexpr (StringMap<T>)
            {
                for (auto& item : value.as_object())
                {
                    normalizeJsonIntegers<typename T::mapped_type>(item.value());
                }
            }
            else if constexpr (awl::inserter_defined<T>)
            {
                for (auto& item : value.as_array())
                {
                    normalizeJsonIntegers<typename T::value_type>(item);
                }
            }
            else if constexpr (TupleTraits<T>::value)
            {
                [&]<std::size_t... Indices>(std::index_sequence<Indices...>)
                {
                    (normalizeJsonIntegers<std::remove_cvref_t<std::tuple_element_t<Indices, T>>>(value.as_array()[Indices]), ...);
                }(std::make_index_sequence<std::tuple_size_v<T>>{});
            }
            else if constexpr (awl::reflectable<T>)
            {
                forEachField<T>([&]<class Field>(const std::string& name)
                {
                    if (boost::json::value* field = value.as_object().if_contains(name))
                    {
                        normalizeJsonIntegers<Field>(*field);
                    }
                });
            }
        }
    }

    template <class T>
    boost::json::object makeJsonSchema()
    {
        json_schema_detail::SchemaContext context;
        boost::json::object schema = json_schema_detail::buildJsonSchema<std::remove_cvref_t<T>>(context);

        if (!context.definitions.empty())
        {
            schema["$defs"] = std::move(context.definitions);
        }

        schema["$schema"] = "https://json-schema.org/draft/2020-12/schema";
        return schema;
    }

    // Validates the supported C++ type shapes, not arbitrary JSON Schema documents.
    template <class T>
    void validateJson(const boost::json::value& value)
    {
        json_schema_detail::validateJsonDocument(value, "$");
        json_schema_detail::validateJsonValue<std::remove_cvref_t<T>>(value, "$");
    }

    // Call after validateJson<T>() and before fromJson() for typed integer fields.
    template <class T>
    void normalizeJsonIntegers(boost::json::value& value)
    {
        json_schema_detail::normalizeJsonIntegers<std::remove_cvref_t<T>>(value);
    }
}
