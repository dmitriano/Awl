/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <array>
#include <concepts>
#include <cstddef>
#include <cstring>
#include <functional>
#include <memory>
#include <type_traits>
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

        template <class Callable>
            requires (
                !std::is_same_v<std::remove_cvref_t<Callable>, equatable_function> &&
                std::copy_constructible<std::decay_t<Callable>> &&
                std::is_invocable_r_v<Result, std::decay_t<Callable>&, Args...>
            )
        equatable_function(Callable&& callable)
            : m_invocable(std::make_unique<ErasedCallable<std::decay_t<Callable>>>(std::forward<Callable>(callable)))
        {
        }

        template <class Object, class Member>
            requires (
                std::is_member_function_pointer_v<Member> &&
                std::is_invocable_r_v<Result, Member, Object*, Args...>
            )
        equatable_function(Object* p_object, Member member)
        {
            m_invocable = std::make_unique<ErasedMember<Object, Member>>(p_object, member);
        }

        template <class Object, class Member>
            requires (
                std::is_member_function_pointer_v<Member> &&
                std::is_invocable_r_v<Result, Member, Object*, Args...>
            )
        equatable_function(Object& object, Member member)
        {
            m_invocable = std::make_unique<ErasedMember<Object, Member>>(std::addressof(object), member);
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
            virtual bool equals(const Invocable& other) const noexcept = 0;
            virtual std::size_t hash() const noexcept = 0;
            virtual std::unique_ptr<Invocable> clone() const = 0;
        };

        template <class T>
        static void combine_hash(std::size_t& seed, const T& val) noexcept
        {
            seed ^= std::hash<T>{}(val) + 0x9e3779b9u + (seed << 6) + (seed >> 2);
        }

        template <class Value>
        static constexpr bool has_equality_operator =
            requires (const Value& left, const Value& right)
            {
                { left == right } -> std::convertible_to<bool>;
            };

        template <class Value>
        static constexpr bool has_std_hash =
            requires (const Value& val)
            {
                { std::hash<Value>{}(val) } -> std::convertible_to<std::size_t>;
            };

        template <class Value>
        static void combine_binary_hash(std::size_t& seed, const Value& val) noexcept
        {
            std::array<std::byte, sizeof(Value)> bytes{};
            std::memcpy(bytes.data(), std::addressof(val), sizeof(Value));

            for (const std::byte b : bytes)
            {
                combine_hash(seed, static_cast<unsigned int>(b));
            }
        }

        template <class Callable>
        class ErasedCallable final : public Invocable
        {
        public:

            explicit ErasedCallable(Callable callable)
                : m_callable(std::move(callable))
            {
            }

            Result invoke(Args... args) const override
            {
                return std::invoke(m_callable, std::forward<Args>(args)...);
            }

            bool equals(const Invocable& other) const noexcept override
            {
                if (this == std::addressof(other))
                {
                    return true;
                }

                const auto* p_other = dynamic_cast<const ErasedCallable*>(&other);

                if (p_other == nullptr)
                {
                    return false;
                }

                if constexpr (has_equality_operator<Callable>)
                {
                    return m_callable == p_other->m_callable;
                }
                else
                {
                    return false;
                }
            }

            std::size_t hash() const noexcept override
            {
                std::size_t seed = 0;
                combine_hash(seed, std::type_index(typeid(Callable)));

                if constexpr (has_std_hash<Callable>)
                {
                    combine_hash(seed, std::hash<Callable>{}(m_callable));
                }

                return seed;
            }

            std::unique_ptr<Invocable> clone() const override
            {
                return std::make_unique<ErasedCallable>(*this);
            }

        private:

            mutable Callable m_callable;
        };

        template <class Object, class Member>
        class ErasedMember final : public Invocable
        {
        public:

            ErasedMember(Object* p_object, Member member)
                : m_object(p_object)
                , m_member(member)
            {
            }

            Result invoke(Args... args) const override
            {
                return std::invoke(m_member, m_object, std::forward<Args>(args)...);
            }

            bool equals(const Invocable& other) const noexcept override
            {
                if (this == std::addressof(other))
                {
                    return true;
                }

                const auto* p_other = dynamic_cast<const ErasedMember*>(&other);
                return p_other != nullptr && m_object == p_other->m_object && m_member == p_other->m_member;
            }

            std::size_t hash() const noexcept override
            {
                std::size_t seed = 0;
                combine_hash(seed, std::type_index(typeid(std::remove_cv_t<Object>)));
                combine_hash(seed, static_cast<const void*>(m_object));
                combine_hash(seed, std::type_index(typeid(Member)));
                combine_binary_hash(seed, m_member);
                return seed;
            }

            std::unique_ptr<Invocable> clone() const override
            {
                return std::make_unique<ErasedMember>(*this);
            }

        private:

            Object* m_object = nullptr;
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
