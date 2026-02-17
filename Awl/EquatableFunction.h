/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <cstddef>
#include <cstring>
#include <functional>
#include <memory>
#include <type_traits>
#include <typeindex>
#include <utility>
#include <vector>

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

        template <class Callable>
            requires (
                !std::is_same_v<std::remove_cvref_t<Callable>, equatable_function> &&
                std::is_invocable_r_v<Result, Callable&, Args...>
            )
        equatable_function(Callable&& callable)
            : m_function(std::forward<Callable>(callable))
        {
        }

        template <class Object, class Member>
            requires (
                std::is_member_function_pointer_v<Member> &&
                std::is_invocable_r_v<Result, Member, Object*, Args...>
            )
        equatable_function(Object* p_object, Member member)
        {
            assign(p_object, member);
        }

        template <class Object, class Member>
            requires (
                std::is_member_function_pointer_v<Member> &&
                std::is_invocable_r_v<Result, Member, Object*, Args...>
            )
        equatable_function(Object& object, Member member)
        {
            assign(std::addressof(object), member);
        }

        Result operator()(Args... args) const
        {
            return m_function(std::forward<Args>(args)...);
        }

        explicit operator bool() const noexcept
        {
            return static_cast<bool>(m_function);
        }

        bool has_identity() const noexcept
        {
            return m_has_identity;
        }

        std::size_t hash() const noexcept
        {
            std::size_t seed = 0;

            combine_hash(seed, m_has_identity);

            if (!m_has_identity)
            {
                combine_hash(seed, static_cast<bool>(m_function));
                return seed;
            }

            combine_hash(seed, m_object_type);
            combine_hash(seed, m_object_ptr);
            combine_hash(seed, m_member_type);
            combine_range_hash(seed, m_member_ptr_bytes);

            return seed;
        }

        friend bool operator==(const equatable_function& left, const equatable_function& right) noexcept
        {
            if (!left.m_has_identity || !right.m_has_identity)
            {
                return !left.m_function && !right.m_function;
            }

            return
                left.m_object_type == right.m_object_type &&
                left.m_object_ptr == right.m_object_ptr &&
                left.m_member_type == right.m_member_type &&
                left.m_member_ptr_bytes == right.m_member_ptr_bytes;
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

        template <class T>
        static void combine_hash(std::size_t& seed, const T& val) noexcept
        {
            seed ^= std::hash<T>{}(val) + 0x9e3779b9u + (seed << 6) + (seed >> 2);
        }

        static void combine_range_hash(std::size_t& seed, const std::vector<std::byte>& bytes) noexcept
        {
            for (std::byte b : bytes)
            {
                combine_hash(seed, static_cast<unsigned int>(b));
            }
        }

        template <class Object, class Member>
        void assign(Object* p_object, Member member)
        {
            m_function = [p_object, member](Args... args) -> Result
            {
                return std::invoke(member, p_object, std::forward<Args>(args)...);
            };

            m_has_identity = true;
            m_object_type = std::type_index(typeid(std::remove_cv_t<Object>));
            m_object_ptr = static_cast<const void*>(p_object);
            m_member_type = std::type_index(typeid(Member));

            m_member_ptr_bytes.resize(sizeof(Member));
            std::memcpy(m_member_ptr_bytes.data(), std::addressof(member), sizeof(Member));
        }

        std::function<Result(Args...)> m_function;
        bool m_has_identity = false;

        std::type_index m_object_type = std::type_index(typeid(void));
        const void* m_object_ptr = nullptr;
        std::type_index m_member_type = std::type_index(typeid(void));
        std::vector<std::byte> m_member_ptr_bytes;
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
