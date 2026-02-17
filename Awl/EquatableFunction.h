/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <array>
#include <bit>
#include <cstddef>
#include <functional>
#include <memory>
#include <new>
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

        equatable_function(std::nullptr_t) noexcept
        {
        }

        equatable_function(const equatable_function& other)
        {
            copy_from(other);
        }

        equatable_function(equatable_function&& other) noexcept
        {
            move_from(std::move(other));
        }

        equatable_function& operator=(const equatable_function& other)
        {
            if (this != std::addressof(other))
            {
                reset();
                copy_from(other);
            }

            return *this;
        }

        equatable_function& operator=(equatable_function&& other) noexcept
        {
            if (this != std::addressof(other))
            {
                reset();
                move_from(std::move(other));
            }

            return *this;
        }

        ~equatable_function()
        {
            reset();
        }

        template <class Object>
        equatable_function(Object* p_object, Result (Object::*member)(Args...))
        {
            emplace_invocable<ErasedMember<decltype(member)>>(p_object, member);
        }

        template <class Object>
        equatable_function(const Object* p_object, Result (Object::*member)(Args...) const)
        {
            emplace_invocable<ErasedMember<decltype(member)>>(p_object, member);
        }

        template <class Object>
        equatable_function(Object& object, Result (Object::*member)(Args...))
        {
            emplace_invocable<ErasedMember<decltype(member)>>(std::addressof(object), member);
        }

        template <class Object>
        equatable_function(const Object& object, Result (Object::*member)(Args...) const)
        {
            emplace_invocable<ErasedMember<decltype(member)>>(std::addressof(object), member);
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
            bool operator==(const Invocable&) const noexcept { return true; }

            virtual Result invoke(Args... args) const = 0;
            virtual bool equals(const Invocable& other) const noexcept = 0;
            virtual std::size_t hash() const noexcept = 0;
            virtual Invocable* clone_to(void* p_storage) const = 0;
            virtual Invocable* move_to(void* p_storage) noexcept = 0;
        };

        template <class T>
        static void combine_hash(std::size_t& seed, const T& val) noexcept
        {
            seed ^= std::hash<T>{}(val) + 0x9e3779b9u + (seed << 6) + (seed >> 2);
        }

        template <class Member>
        static std::size_t member_identity(Member member) noexcept
        {
            static_assert(sizeof(Member) <= sizeof(std::size_t));

            const auto bytes = std::bit_cast<std::array<std::byte, sizeof(Member)>>(member);

            std::size_t value = 0;
            for (std::size_t i = 0; i < sizeof(Member); ++i)
            {
                value |= static_cast<std::size_t>(std::to_integer<unsigned char>(bytes[i])) << (8u * i);
            }

            return value;
        }

        template <class Member>
        struct member_function_traits;

        template <class Object>
        struct member_function_traits<Result (Object::*)(Args...)>
        {
            using object_type = Object;
            using object_ptr = Object*;
        };

        template <class Object>
        struct member_function_traits<Result (Object::*)(Args...) const>
        {
            using object_type = Object;
            using object_ptr = const Object*;
        };

        template <class Member>
        class ErasedMember final : public Invocable
        {
        public:

            using Traits = member_function_traits<Member>;
            using Object = typename Traits::object_type;
            using ObjectPtr = typename Traits::object_ptr;

            ErasedMember(ObjectPtr p_object, Member member)
                : m_object(p_object)
                , m_member(member)
            {
            }

            bool operator==(const ErasedMember& other) const = default;

            Result invoke(Args... args) const override
            {
                return std::invoke(m_member, m_object, std::forward<Args>(args)...);
            }

            bool equals(const Invocable& other) const noexcept override
            {
                const auto* p_other = dynamic_cast<const ErasedMember*>(&other);
                return p_other != nullptr && *this == *p_other;
            }

            std::size_t hash() const noexcept override
            {
                std::size_t seed = 0;
                combine_hash(seed, std::type_index(typeid(Object)));
                combine_hash(seed, static_cast<const void*>(m_object));
                combine_hash(seed, member_identity(m_member));
                return seed;
            }

            Invocable* clone_to(void* p_storage) const override
            {
                return ::new (p_storage) ErasedMember(*this);
            }

            Invocable* move_to(void* p_storage) noexcept override
            {
                return ::new (p_storage) ErasedMember(std::move(*this));
            }

        private:

            ObjectPtr m_object = nullptr;
            Member m_member{};
        };

        static constexpr std::size_t storage_size = 3 * sizeof(void*);
        alignas(std::max_align_t) std::byte m_storage[storage_size];
        Invocable* m_invocable = nullptr;

        void* storage_ptr() noexcept
        {
            return static_cast<void*>(m_storage);
        }

        const void* storage_ptr() const noexcept
        {
            return static_cast<const void*>(m_storage);
        }

        void reset() noexcept
        {
            if (m_invocable != nullptr)
            {
                m_invocable->~Invocable();
                m_invocable = nullptr;
            }
        }

        void copy_from(const equatable_function& other)
        {
            if (other.m_invocable != nullptr)
            {
                m_invocable = other.m_invocable->clone_to(storage_ptr());
            }
        }

        void move_from(equatable_function&& other) noexcept
        {
            if (other.m_invocable != nullptr)
            {
                m_invocable = other.m_invocable->move_to(storage_ptr());
                other.reset();
            }
        }

        template <class T, class... Ts>
        void emplace_invocable(Ts&&... args)
        {
            static_assert(sizeof(T) <= storage_size);
            static_assert(alignof(T) <= alignof(std::max_align_t));

            m_invocable = ::new (storage_ptr()) T(std::forward<Ts>(args)...);
        }
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
