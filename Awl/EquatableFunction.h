/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <concepts>
#include <cstdint>
#include <functional>
#include <memory>
#include <new>
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
    private:

        class Invocable;

    public:

        using signature_type = Result(Args...);
        
        class invocation_guard
        {
        public:

            invocation_guard() = default;

            explicit operator bool() const noexcept
            {
                return m_invocable != nullptr;
            }

            Result operator()(Args... args) const
            {
                return invoke(std::forward<Args>(args)...);
            }

        private:

            Result invoke(Args... args) const
            {
                if (!m_invocable)
                {
                    throw std::bad_function_call();
                }

                return m_invocable->invoke_locked(m_owner, std::forward<Args>(args)...);
            }

            invocation_guard(const Invocable* p_invocable, std::shared_ptr<void> owner)
                : m_invocable(p_invocable)
                , m_owner(std::move(owner))
            {
            }

            const Invocable* m_invocable = nullptr;
            std::shared_ptr<void> m_owner;

            friend equatable_function;
        };

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
        equatable_function(const std::shared_ptr<Object>& p_object, Result (Object::*member)(Args...))
            : equatable_function(std::weak_ptr<Object>(p_object), member)
        {
        }

        template <class Object>
        equatable_function(const std::shared_ptr<Object>& p_object, Result (Object::*member)(Args...) const)
            : equatable_function(std::weak_ptr<Object>(p_object), member)
        {
        }

        template <class Object>
        equatable_function(std::weak_ptr<Object> p_object, Result (Object::*member)(Args...))
        {
            emplace_invocable<ErasedWeak<decltype(member)>>(std::move(p_object), member);
        }

        template <class Object>
        equatable_function(std::weak_ptr<Object> p_object, Result (Object::*member)(Args...) const)
        {
            emplace_invocable<ErasedWeak<decltype(member)>>(std::move(p_object), member);
        }

        equatable_function(std::uint64_t id, std::function<Result(Args...)> func)
        {
            emplace_invocable<ErasedLambda>(id, std::move(func));
        }

        Result operator()(Args... args) const
        {
            if (!m_invocable)
            {
                throw std::bad_function_call();
            }

            return m_invocable->invoke(std::forward<Args>(args)...);
        }

        [[nodiscard]] invocation_guard lock() const noexcept
        {
            if (!m_invocable)
            {
                return {};
            }

            std::shared_ptr<void> owner;

            if (!m_invocable->try_lock(owner))
            {
                return {};
            }

            return invocation_guard(m_invocable, std::move(owner));
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
            virtual bool try_lock(std::shared_ptr<void>& owner) const noexcept = 0;
            virtual Result invoke_locked(const std::shared_ptr<void>& owner, Args... args) const = 0;
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

        template <class Object, class Member>
        static std::size_t compute_hash(const void* p_object, const Member& member) noexcept
        {
            std::size_t seed = 0;
            combine_hash(seed, std::type_index(typeid(Object)));
            combine_hash(seed, p_object);

            const auto bytes = std::bit_cast<std::array<std::byte, sizeof(Member)>>(member);

            constexpr std::size_t chunk_size = sizeof(std::size_t);
            const std::size_t chunk_count = bytes.size() / chunk_size;

            for (std::size_t i = 0; i < chunk_count; ++i)
            {
                std::array<std::byte, chunk_size> chunk_bytes{};
                const auto first = bytes.begin() + static_cast<std::ptrdiff_t>(i * chunk_size);
                const auto last = first + static_cast<std::ptrdiff_t>(chunk_size);
                std::copy(first, last, chunk_bytes.begin());

                combine_hash(seed, std::bit_cast<std::size_t>(chunk_bytes));
            }

            const std::size_t remainder_begin = chunk_count * chunk_size;

            for (std::size_t i = remainder_begin; i < bytes.size(); ++i)
            {
                combine_hash(seed, std::to_integer<unsigned int>(bytes[i]));
            }

            return seed;
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

        class ErasedLambda final : public Invocable
        {
        public:

            ErasedLambda(std::uint64_t id, std::function<Result(Args...)> func)
                : m_id(id)
                , m_func(std::move(func))
            {
            }

            bool operator==(const ErasedLambda& other) const noexcept
            {
                return m_id == other.m_id;
            }

            Result invoke(Args... args) const override
            {
                return std::invoke(m_func, std::forward<Args>(args)...);
            }

            bool try_lock(std::shared_ptr<void>&) const noexcept override
            {
                return true;
            }

            Result invoke_locked(const std::shared_ptr<void>&, Args... args) const override
            {
                return invoke(std::forward<Args>(args)...);
            }

            bool equals(const Invocable& other) const noexcept override
            {
                const auto* p_other = dynamic_cast<const ErasedLambda*>(&other);
                return p_other != nullptr && m_id == p_other->m_id;
            }

            std::size_t hash() const noexcept override
            {
                std::size_t seed = 0;
                combine_hash(seed, m_id);
                return seed;
            }

            Invocable* clone_to(void* p_storage) const override
            {
                return ::new (p_storage) ErasedLambda(*this);
            }

            Invocable* move_to(void* p_storage) noexcept override
            {
                return ::new (p_storage) ErasedLambda(std::move(*this));
            }

        private:

            std::uint64_t m_id = 0;
            mutable std::function<Result(Args...)> m_func;
        };

        template <class Member>
        class ErasedWeak final : public Invocable
        {
        public:

            using Traits = member_function_traits<Member>;
            using Object = typename Traits::object_type;
            using WeakObject = std::remove_const_t<Object>;
            using WeakPtr = std::weak_ptr<WeakObject>;

            ErasedWeak(WeakPtr p_object, Member member)
                : m_object(std::move(p_object))
                , m_member(member)
            {
            }

            bool operator==(const ErasedWeak& other) const
            {
                return object_ptr() == other.object_ptr() && m_member == other.m_member;
            }

            Result invoke(Args... args) const override
            {
                std::shared_ptr<WeakObject> p_object = m_object.lock();

                if (!p_object)
                {
                    throw std::bad_function_call();
                }

                return std::invoke(m_member, p_object.get(), std::forward<Args>(args)...);
            }
            
            bool try_lock(std::shared_ptr<void>& owner) const noexcept override
            {
                owner = m_object.lock();
                return static_cast<bool>(owner);
            }

            Result invoke_locked(const std::shared_ptr<void>& owner, Args... args) const override
            {
                auto* p_object = static_cast<WeakObject*>(owner.get());

                if (p_object == nullptr)
                {
                    throw std::bad_function_call();
                }

                return std::invoke(m_member, p_object, std::forward<Args>(args)...);
            }

            bool equals(const Invocable& other) const noexcept override
            {
                const auto* p_other = dynamic_cast<const ErasedWeak*>(&other);
                return p_other != nullptr && *this == *p_other;
            }

            std::size_t hash() const noexcept override
            {
                return compute_hash<Object>(object_ptr(), m_member);
            }

            Invocable* clone_to(void* p_storage) const override
            {
                return ::new (p_storage) ErasedWeak(*this);
            }

            Invocable* move_to(void* p_storage) noexcept override
            {
                return ::new (p_storage) ErasedWeak(std::move(*this));
            }

        private:
            const void* object_ptr() const noexcept
            {
                std::shared_ptr<WeakObject> p_locked = m_object.lock();
                return p_locked ? static_cast<const void*>(p_locked.get()) : nullptr;
            }

            WeakPtr m_object;
            Member m_member{};
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
            
            bool try_lock(std::shared_ptr<void>& owner) const noexcept override
            {
                owner.reset();
                return true;
            }

            Result invoke_locked(const std::shared_ptr<void>&, Args... args) const override
            {
                return invoke(std::forward<Args>(args)...);
            }

            bool equals(const Invocable& other) const noexcept override
            {
                const auto* p_other = dynamic_cast<const ErasedMember*>(&other);
                return p_other != nullptr && *this == *p_other;
            }

            std::size_t hash() const noexcept override
            {
                return compute_hash<Object>(static_cast<const void*>(m_object), m_member);
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

        struct HandleSample { void f() {} };

        // The size of the pointer to member function is 1 pointer in MSVC and 2 pointers in GCC on x64.
        // The last void* is vtable.
        static constexpr std::size_t member_storage_size =
            std::max(
                sizeof(std::uint64_t) + sizeof(std::function<Result(Args...)>),
                sizeof(std::weak_ptr<HandleSample>) + sizeof(void (HandleSample::*)())) +
            sizeof(void*);

        static constexpr std::size_t storage_size = std::max(member_storage_size, sizeof(ErasedLambda));
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
