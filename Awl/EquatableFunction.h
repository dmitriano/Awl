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
                return _invocable != nullptr;
            }

            Result operator()(Args... args) const
            {
                return invoke(std::forward<Args>(args)...);
            }

        private:

            Result invoke(Args... args) const
            {
                if (!_invocable)
                {
                    throw std::bad_function_call();
                }

                return _invocable->invoke_locked(_owner, std::forward<Args>(args)...);
            }

            invocation_guard(const Invocable* p_invocable, std::shared_ptr<void> owner)
                : _invocable(p_invocable)
                , _owner(std::move(owner))
            {}

            const Invocable* _invocable = nullptr;
            std::shared_ptr<void> _owner;

            friend equatable_function;
        };

        equatable_function() = default;

        equatable_function(std::nullptr_t) noexcept
        {}

        equatable_function(const equatable_function& other) = delete;

        equatable_function(equatable_function&& other) noexcept
        {
            move_from(std::move(other));
        }

        equatable_function& operator=(const equatable_function& other) = delete;

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
        equatable_function(std::shared_ptr<Object> p_object, Result (Object::*member)(Args...))
        {
            emplace_invocable<ErasedShared<decltype(member)>>(std::move(p_object), member);
        }

        template <class Object>
        equatable_function(std::shared_ptr<Object> p_object, Result (Object::*member)(Args...) const)
        {
            emplace_invocable<ErasedShared<decltype(member)>>(std::move(p_object), member);
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

        equatable_function(std::uint64_t id, std::move_only_function<Result(Args...)> func)
        {
            emplace_invocable<ErasedLambda>(id, std::move(func));
        }

        Result operator()(Args... args) const
        {
            if (!_invocable)
            {
                throw std::bad_function_call();
            }

            return _invocable->invoke(std::forward<Args>(args)...);
        }

        [[nodiscard]] invocation_guard lock() const noexcept
        {
            if (!_invocable)
            {
                return {};
            }

            std::shared_ptr<void> owner;

            if (!_invocable->try_lock(owner))
            {
                return {};
            }

            return invocation_guard(_invocable, std::move(owner));
        }

        explicit operator bool() const noexcept
        {
            return static_cast<bool>(_invocable);
        }

        std::size_t hash() const noexcept
        {
            return _invocable ? _invocable->hash() : std::hash<std::size_t>{}(0u);
        }

        friend bool operator==(const equatable_function& left, const equatable_function& right) noexcept
        {
            if (!left._invocable || !right._invocable)
            {
                return !left._invocable && !right._invocable;
            }

            return left._invocable->equals(*right._invocable);
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
            virtual void destroy() noexcept = 0;
            virtual Invocable* move_to(void* p_storage) noexcept = 0;
        };

        template <class Derived>
        class InvocableImpl : public Invocable
        {
        public:

            bool try_lock(std::shared_ptr<void>& owner) const noexcept override
            {
                owner.reset();
                return true;
            }

            Result invoke_locked(const std::shared_ptr<void>&, Args... args) const override
            {
                return static_cast<const Derived&>(*this).invoke(std::forward<Args>(args)...);
            }

            bool equals(const Invocable& other) const noexcept override
            {
                const auto* p_other = dynamic_cast<const Derived*>(&other);
                return p_other != nullptr && static_cast<const Derived&>(*this) == *p_other;
            }

            void destroy() noexcept override
            {
                static_cast<Derived*>(this)->~Derived();
            }

            Invocable* move_to(void* p_storage) noexcept override
            {
                return ::new (p_storage) Derived(std::move(static_cast<Derived&>(*this)));
            }

        protected:

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
        };

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

        class ErasedLambda final : public InvocableImpl<ErasedLambda>
        {
        public:

            ErasedLambda(std::uint64_t id, std::move_only_function<Result(Args...)> func)
                : _id(id)
                , _func(std::move(func))
            {}

            bool operator==(const ErasedLambda& other) const noexcept
            {
                return _id == other._id;
            }

            Result invoke(Args... args) const override
            {
                return std::invoke(_func, std::forward<Args>(args)...);
            }

            std::size_t hash() const noexcept override
            {
                return std::hash<std::uint64_t>{}(_id);
            }

        private:

            std::uint64_t _id = 0;
            mutable std::move_only_function<Result(Args...)> _func;
        };

        template <class Member>
        class ErasedWeak final : public InvocableImpl<ErasedWeak<Member>>
        {
        public:

            using Traits = member_function_traits<Member>;
            using Object = typename Traits::object_type;
            using WeakObject = std::remove_const_t<Object>;
            using WeakPtr = std::weak_ptr<WeakObject>;

            ErasedWeak(WeakPtr p_object, Member member)
                : _object(std::move(p_object))
                , _member(member)
            {}

            bool operator==(const ErasedWeak& other) const
            {
                return object_ptr() == other.object_ptr() && _member == other._member;
            }

            Result invoke(Args... args) const override
            {
                std::shared_ptr<WeakObject> p_object = _object.lock();

                if (!p_object)
                {
                    throw std::bad_function_call();
                }

                return std::invoke(_member, p_object.get(), std::forward<Args>(args)...);
            }
            
            bool try_lock(std::shared_ptr<void>& owner) const noexcept override
            {
                owner = _object.lock();
                return static_cast<bool>(owner);
            }

            Result invoke_locked(const std::shared_ptr<void>& owner, Args... args) const override
            {
                auto* p_object = static_cast<WeakObject*>(owner.get());

                if (p_object == nullptr)
                {
                    throw std::bad_function_call();
                }

                return std::invoke(_member, p_object, std::forward<Args>(args)...);
            }

            std::size_t hash() const noexcept override
            {
                return InvocableImpl<ErasedWeak<Member>>::template compute_hash<Object>(object_ptr(), _member);
            }

        private:

            const void* object_ptr() const noexcept
            {
                std::shared_ptr<WeakObject> p_locked = _object.lock();
                return p_locked ? static_cast<const void*>(p_locked.get()) : nullptr;
            }

            WeakPtr _object;
            Member _member{};
        };

        template <class Member>
        class ErasedShared final : public InvocableImpl<ErasedShared<Member>>
        {
        public:

            using Traits = member_function_traits<Member>;
            using Object = typename Traits::object_type;
            using SharedObject = std::remove_const_t<Object>;
            using SharedPtr = std::shared_ptr<SharedObject>;

            ErasedShared(SharedPtr p_object, Member member)
                : _object(std::move(p_object))
                , _member(member)
            {}

            bool operator==(const ErasedShared& other) const = default;

            Result invoke(Args... args) const override
            {
                if (!_object)
                {
                    throw std::bad_function_call();
                }

                return std::invoke(_member, _object, std::forward<Args>(args)...);
            }

            std::size_t hash() const noexcept override
            {
                if (!_object)
                {
                    return std::hash<std::size_t>{}(0u);
                }

                return InvocableImpl<ErasedShared<Member>>::template compute_hash<Object>(static_cast<const void*>(_object.get()), _member);
            }

        private:

            SharedPtr _object;
            Member _member{};
        };

        template <class Member>
        class ErasedMember final : public InvocableImpl<ErasedMember<Member>>
        {
        public:

            using Traits = member_function_traits<Member>;
            using Object = typename Traits::object_type;
            using ObjectPtr = typename Traits::object_ptr;

            ErasedMember(ObjectPtr p_object, Member member)
                : _object(p_object)
                , _member(member)
            {}

            bool operator==(const ErasedMember& other) const = default;

            Result invoke(Args... args) const override
            {
                return std::invoke(_member, _object, std::forward<Args>(args)...);
            }

            std::size_t hash() const noexcept override
            {
                return InvocableImpl<ErasedMember<Member>>::template compute_hash<Object>(static_cast<const void*>(_object), _member);
            }

        private:

            ObjectPtr _object = nullptr;
            Member _member{};
        };

        struct HandleSample { void f() {} };

        // The size of the pointer to member function is 1 pointer in MSVC and 2 pointers in GCC on x64.
        // The last void* is vtable.
        static constexpr std::size_t member_storage_size =
            std::max(
                sizeof(std::uint64_t) + sizeof(std::move_only_function<Result(Args...)>),
                std::max(
                    sizeof(std::weak_ptr<HandleSample>) + sizeof(void (HandleSample::*)()),
                    sizeof(std::shared_ptr<HandleSample>) + sizeof(void (HandleSample::*)()))) +
            sizeof(void*);

        static constexpr std::size_t storage_size = std::max(member_storage_size, sizeof(ErasedLambda));
        alignas(std::max_align_t) std::byte _storage[storage_size];
        Invocable* _invocable = nullptr;

        void* storage_ptr() noexcept
        {
            return static_cast<void*>(_storage);
        }

        const void* storage_ptr() const noexcept
        {
            return static_cast<const void*>(_storage);
        }

        void reset() noexcept
        {
            if (_invocable != nullptr)
            {
                _invocable->destroy();
                _invocable = nullptr;
            }
        }

        void move_from(equatable_function&& other) noexcept
        {
            if (other._invocable != nullptr)
            {
                _invocable = other._invocable->move_to(storage_ptr());
                other._invocable->destroy();
                other._invocable = nullptr;
            }
        }

        template <class T, class... Ts>
        void emplace_invocable(Ts&&... args)
        {
            static_assert(sizeof(T) <= storage_size);
            static_assert(alignof(T) <= alignof(std::max_align_t));

            _invocable = ::new (storage_ptr()) T(std::forward<Ts>(args)...);
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
