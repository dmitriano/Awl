#pragma once

#include <type_traits>

namespace awl
{
    template<std::size_t N>
    struct FixedString
    {
        char state[N + 1] = { 0 };
        
        constexpr FixedString(char const(&arr)[N + 1])
        {
            for (std::size_t i = 0; i < N; ++i)
                state[i] = arr[i];
        }
        
        constexpr char operator[](std::size_t i) const { return state[i]; }
        constexpr char& operator[](std::size_t i) { return state[i]; }

        constexpr explicit operator char const*() const { return state; }
        constexpr char const* data() const { return state; }
        constexpr std::size_t size() const { return N; }
        constexpr char const* begin() const { return state; }
        constexpr char const* end() const { return begin() + size(); }

        constexpr FixedString() = default;
        constexpr FixedString(FixedString const&) = default;
        constexpr FixedString& operator=(FixedString const&) = default;

        template<std::size_t M>
        friend constexpr FixedString<N + M> operator+(FixedString lhs, FixedString<M> rhs)
        {
            FixedString<N + M> retval;
            for (std::size_t i = 0; i < N; ++i)
                retval[i] = lhs[i];
            for (std::size_t i = 0; i < M; ++i)
                retval[N + i] = rhs[i];
            return retval;
        }

        friend constexpr bool operator==(FixedString lhs, FixedString rhs)
        {
            for (std::size_t i = 0; i < N; ++i)
                if (lhs[i] != rhs[i]) return false;
            return true;
        }

        friend constexpr bool operator!=(FixedString lhs, FixedString rhs)
        {
            for (std::size_t i = 0; i < N; ++i)
                if (lhs[i] != rhs[i]) return true;
            return false;
        }
        
        template<std::size_t M, std::enable_if_t<M != N, bool> = true>
        friend constexpr bool operator!=(FixedString, FixedString<M>)
        {
            return true;
        }

        template<std::size_t M, std::enable_if_t<M != N, bool> = true>
        friend constexpr bool operator==(FixedString, FixedString<M>)
        {
            return false;
        }
    };

    template<std::size_t N>
    FixedString(char const(&)[N])->FixedString<N - 1>;
}
