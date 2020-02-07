#pragma once

namespace awl
{
    template <class T> class EnumTraits;
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
    };
