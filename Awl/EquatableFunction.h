/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <typeindex>
#include <utility>

namespace awl
{
    template <class Signature>
    class equatable_function;

    template <class Result, class... Args>
    class equatable_function<Result(Args...)>
    {
    public:

        using signature_type = Result(Args...);

        struct TargetInfo
        {
            std::type_index Type = std::type_index(typeid(void));
            const void* Object = nullptr;
            const void* Member = nullptr;

            bool operator==(const TargetInfo&) const = default;
        };

        equatable_function() = default;
        ~equatable_function() = default;

        equatable_function(std::nullptr_t) noexcept
        {
        }

        equatable_function(const equatable_function& other)
        {
            if (other.m_invocable)
            {
                m_invocable = other.m_invocable->clone();
            }
        }

        equatable_function(equatable_function&& other) noexcept = default;

        equatable_function& operator=(const equatable_function& other)
        {
            if (this != std::addressof(other))
            {
                m_invocable = other.m_invocable ? other.m_invocable->clone() : nullptr;
            }

            return *this;
        }

        equatable_function& operator=(equatable_function&& other) noexcept = default;

        template <class Object>
        equatable_function(Object* p_object, Result (Object::*member)(Args...))
        {
            m_invocable = std::make_unique<ErasedMember<decltype(member)>>(p_object, member);
        }

        template <class Object>
        equatable_function(const Object* p_object, Result (Object::*member)(Args...) const)
        {
            m_invocable = std::make_unique<ErasedMember<decltype(member)>>(p_object, member);
        }

        template <class Object>
        equatable_function(Object& object, Result (Object::*member)(Args...))
        {
            m_invocable = std::make_unique<ErasedMember<decltype(member)>>(std::addressof(object), member);
        }

        template <class Object>
        equatable_function(const Object& object, Result (Object::*member)(Args...) const)
        {
            m_invocable = std::make_unique<ErasedMember<decltype(member)>>(std::addressof(object), member);
        }

        Result operator()(Args... args) const
        {
            if (!m_invocable)
            {
                throw std::bad_function_call();
            }

            return m_invocable->invoke(std::forward<Args>(args)...);
        }

        explicit operator bool() const noexcept
        {
            return static_cast<bool>(m_invocable);
        }

        std::size_t hash() const noexcept
        {
            return m_invocable ? m_invocable->hash() : 0u;
        }

        friend bool operator==(const equatable_function& left, const equatable_function& right) noexcept
        {
            if (!left.m_invocable || !right.m_invocable)
            {
                return !left.m_invocable && !right.m_invocable;
            }

            return left.m_invocable->equals(*right.m_invocable);
        }

        friend bool operator!=(const equatable_function& left, const equatable_function& right) noexcept
        {
            return !(left == right);
        }

        friend bool operator==(const equatable_function& f, std::nullptr_t) noexcept
        {
            return !f;
        }

        friend bool operator==(std::nullptr_t, const equatable_function& f) noexcept
        {
            return !f;
        }

    private:

        class Invocable
        {
        public:

            virtual ~Invocable() = default;

            virtual Result invoke(Args... args) const = 0;
            virtual TargetInfo target_info() const noexcept = 0;
            bool equals(const Invocable& other) const noexcept
            {
                return target_info() == other.target_info();
            }
            virtual std::size_t hash() const noexcept = 0;
            virtual std::unique_ptr<Invocable> clone() const = 0;
        };

        template <class T>
        static void combine_hash(std::size_t& seed, const T& val) noexcept
        {
            seed ^= std::hash<T>{}(val) + 0x9e3779b9u + (seed << 6) + (seed >> 2);
        }

        template <class Member>
        static const void* member_identity(Member member) noexcept
        {
            static_assert(sizeof(Member) <= sizeof(std::uintptr_t));

            const auto bytes = std::bit_cast<std::array<std::byte, sizeof(Member)>>(member);

            std::uintptr_t value = 0;
            for (std::size_t i = 0; i < sizeof(Member); ++i)
            {
                value |= static_cast<std::uintptr_t>(std::to_integer<unsigned char>(bytes[i])) << (8u * i);
            }

            return reinterpret_cast<const void*>(value);
        }

        template <class Member>
        struct member_fn_traits;

        template <class Object>
        struct member_fn_traits<Result (Object::*)(Args...)>
        {
            using object_type = Object;
            using object_ptr = Object*;
        };

        template <class Object>
        struct member_fn_traits<Result (Object::*)(Args...) const>
        {
            using object_type = Object;
            using object_ptr = const Object*;
        };

        template <class Member>
        class ErasedMember;

        template <class Member>
        class ErasedMember final : public Invocable
        {
        public:

            using Traits = member_fn_traits<Member>;
            using Object = typename Traits::object_type;
            using ObjectPtr = typename Traits::object_ptr;

            ErasedMember(ObjectPtr p_object, Member member)
                : m_object(p_object)
                , m_member(member)
            {
            }

            Result invoke(Args... args) const override
            {
                return std::invoke(m_member, m_object, std::forward<Args>(args)...);
            }

            TargetInfo target_info() const noexcept override
            {
                return
                {
                    std::type_index(typeid(Object)),
                    static_cast<const void*>(m_object),
                    member_identity(m_member)
                };
            }

            std::size_t hash() const noexcept override
            {
                std::size_t seed = 0;
                const auto info = target_info();
                combine_hash(seed, info.Type);
                combine_hash(seed, info.Object);
                combine_hash(seed, info.Member);
                return seed;
            }

            std::unique_ptr<Invocable> clone() const override
            {
                return std::make_unique<ErasedMember>(*this);
            }

        private:

            ObjectPtr m_object = nullptr;
            Member m_member{};
        };

        std::unique_ptr<Invocable> m_invocable;
    };
}

namespace std
{
    template <class Result, class... Args>
    struct hash<awl::equatable_function<Result(Args...)>>
    {
        std::size_t operator()(const awl::equatable_function<Result(Args...)>& f) const noexcept
        {
            return f.hash();
        }
    };
}
