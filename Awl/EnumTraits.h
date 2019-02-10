#pragma once

namespace awl
{
    template <class T> struct EnumTraits;
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

//We cannot specialize awl::EnumTraits inside a class or a namespace, so we define EnumName##Traits.
#define AWL_SEQUENTIAL_ENUM_IMPL(EnumName, access, ...) \
    enum class EnumName { __VA_ARGS__ }; \
    access \
    struct EnumName##Traits \
    { \
    private: \
        enum { __VA_ARGS__, Last }; \
    public: \
        static constexpr size_t Count = Last; \
    };

#define AWL_ENCLOSED_SEQUENTIAL_ENUM(EnumName, ...) AWL_SEQUENTIAL_ENUM_IMPL(EnumName, private:, __VA_ARGS__)

#define AWL_SEQUENTIAL_ENUM(EnumName, ...) AWL_SEQUENTIAL_ENUM_IMPL(EnumName, , __VA_ARGS__)

#define AWL_ENUM_TRAITS(ns, EnumName) \
    template<> \
    struct awl::EnumTraits<ns::EnumName> \
    { \
    public: \
        static constexpr std::size_t count() \
        { \
            return ns::EnumName##Traits::Count; \
        } \
    };
