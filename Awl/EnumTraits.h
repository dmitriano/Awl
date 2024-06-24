/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/TypeTraits.h"
#include "Awl/Stringizable.h"
#include "Awl/StringFormat.h"

namespace awl
{
    template <class T> class EnumTraits;

    template <class T>
    concept is_sequential_enum = std::is_enum_v<T> && is_defined_v<EnumTraits<T>>;

    template <class T>
    concept is_nonsequential_enum = std::is_enum_v<T> && !is_defined_v<EnumTraits<T>>;

    template <class T> requires is_sequential_enum<T>
    [[noreturn]]
    void raise_wrong_enum_index(std::underlying_type_t<T> int_val)
    {
        throw std::runtime_error(aformat() << "Wrong " << EnumTraits<T>::enum_name() << " enum index: " << int_val);
    }

    template <class T> requires std::is_enum_v<T>
    constexpr std::underlying_type_t<T> enum_to_underlying(T val)
    {
        return static_cast<std::underlying_type_t<T>>(val);
    }

    template <class T> requires is_sequential_enum<T>
    std::underlying_type_t<T> enum_to_index(T val)
    {
        const auto int_val = enum_to_underlying(val);

        if (int_val >= EnumTraits<T>::count())
        {
            raise_wrong_enum_index<T>(int_val);
        }

        return int_val;
    }

    template <class T> requires is_sequential_enum<T>
    void validate_enum_index(std::underlying_type_t<T> int_val)
    {
        if (int_val >= EnumTraits<T>::count())
        {
            raise_wrong_enum_index<T>(int_val);
        }
    }

    template <class T> requires is_sequential_enum<T>
    T enum_from_index(std::underlying_type_t<T> int_val)
    {
        validate_enum_index<T>(int_val);

        return static_cast<T>(int_val);
    }

    template <class T> requires is_sequential_enum<T>
    std::string enum_to_string(T val)
    {
        return EnumTraits<T>::names()[enum_to_index(val)];
    }

    namespace helpers
    {
        template <class T, class Pred> requires is_sequential_enum<T>
        T enum_from_string_impl(const std::string& s, Pred&& equal)
        {
            auto& names = EnumTraits<T>::names();

            std::underlying_type_t<T> index = 0;

            for (const auto& name : names)
            {
                if (equal(name, s))
                {
                    return static_cast<T>(index);
                }

                ++index;
            }

            throw std::runtime_error(aformat() << "Wrong " << EnumTraits<T>::enum_name() << "enum name: " << s);
        }
    }

    template <class T> requires is_sequential_enum<T>
    T enum_from_string(const std::string& s)
    {
        return helpers::enum_from_string_impl<T>(s, StringEqual<char>());
    }

    template <class T> requires is_sequential_enum<T>
    T enum_from_string_i(const std::string& s)
    {
        return helpers::enum_from_string_impl<T>(s, StringInsensitiveEqual<char>());
    }

    template <class T> requires is_sequential_enum<T>
    [[noreturn]]
    void raise_wrong_enum_value(T val)
    {
        throw std::runtime_error(aformat() << "Wrong " << EnumTraits<T>::enum_name() << " enum value: " << enum_to_underlying(val));
    }

    // We cast a enum value from some int and then validate it.
    template <class T> requires is_sequential_enum<T>
    void validate_enum(T val)
    {
        auto int_val = enum_to_underlying(val);

        if (int_val >= EnumTraits<T>::count())
        {
            raise_wrong_enum_value(val);
        }
    }
}

//We cannot specialize awl::EnumTraits<EnumName> inside a class or a namespace, so we define EnumName##Traits,
//also EnumName##Traits cannot be private, because if it is a template parameter of a public member.
#define AWL_SEQUENTIAL_ENUM(EnumName, ...) \
    enum class EnumName : uint8_t { __VA_ARGS__ }; \
    class EnumName##Traits \
    { \
    public: \
        using size_type = std::underlying_type<EnumName>::type; \
    private: \
        enum : size_type { __VA_ARGS__, Last }; \
    public: \
        static constexpr char EnumNameString[] = #EnumName; \
        static constexpr size_type Count = Last; \
        /*I was unable to make std::vector constexpr even in MSVC 19.29.30133, probably we need to wait a bit.*/ \
        static inline const awl::helpers::MemberList m_ml{#__VA_ARGS__}; \
    };

//This awl::EnumTraits should be specialized at the global namespace level.
#define AWL_ENUM_TRAITS(ns, EnumName) \
    template<> \
    class awl::EnumTraits<ns::EnumName> \
    { \
    public: \
        using size_type = ns::EnumName##Traits::size_type; \
        static constexpr const char * enum_name() \
        { \
            return ns::EnumName##Traits::EnumNameString; \
        } \
        static constexpr ns::EnumName##Traits::size_type count() \
        { \
            return ns::EnumName##Traits::Count; \
        } \
        static const awl::helpers::MemberList& names() \
        { \
            return ns::EnumName##Traits::m_ml; \
        } \
    };
