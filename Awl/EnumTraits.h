/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Awl/Stringizable.h"

namespace awl
{
    template <class T> class EnumTraits;

    template<typename T, typename = void>
    constexpr bool is_defined_v = false;

    template<typename T>
    constexpr bool is_defined_v<T, decltype(typeid(T), void())> = true;

    template <class T>
    constexpr std::enable_if_t<std::is_enum_v<T>, std::underlying_type_t<T>> enum_to_underlying(T val)
    {
        return static_cast<std::underlying_type_t<T>>(val);
    }

    template <class T>
    std::enable_if_t<std::is_enum_v<T>&& is_defined_v<awl::EnumTraits<T>>, size_t> enum_to_index(T val)
    {
        const size_t int_val = static_cast<size_t>(val);

        if (int_val >= awl::EnumTraits<T>::count())
        {
            throw std::runtime_error("Wrong enum index.");
        }

        return int_val;
    }

    template <class T>
    std::enable_if_t<std::is_enum_v<T>&& is_defined_v<awl::EnumTraits<T>>, T> enum_from_index(size_t int_val)
    {
        if (int_val >= awl::EnumTraits<T>::count())
        {
            throw std::runtime_error("Wrong enum index.");
        }

        return static_cast<T>(int_val);
    }

    template <class T>
    std::enable_if_t<std::is_enum_v<T> && is_defined_v<awl::EnumTraits<T>>, std::string> enum_to_string(T val)
    {
        return awl::EnumTraits<T>::names()[enum_to_index(val)];
    }

    template <class T>
    std::enable_if_t<std::is_enum_v<T> && is_defined_v<awl::EnumTraits<T>>, T> enum_from_string(const std::string& s)
    {
        auto& names = awl::EnumTraits<T>::names();

        std::underlying_type_t<T> index = 0;

        for (const auto& name : names)
        {
            if (name == s)
            {
                return static_cast<T>(index);
            }

            ++index;
        }

        throw std::runtime_error("Wrong enum value.");
    }
}

/*
#define AWL_SEQUENTIAL_ENUM(EnumName, ...) \
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
*/

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
        static constexpr ns::EnumName##Traits::size_type count() \
        { \
            return ns::EnumName##Traits::Count; \
        } \
        static const awl::helpers::MemberList& names() \
        { \
            return ns::EnumName##Traits::m_ml; \
        } \
    };
