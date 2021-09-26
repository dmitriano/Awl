/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Product: AWL (A Working Library)
// Author: Dmitriano
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <type_traits>
#include <string>
#include <string_view>

namespace awl
{
    template<typename CharT, std::size_t N>
    class fixed_string
    {
    public:

        constexpr fixed_string(CharT const(&arr)[N + 1])
        {
            for (std::size_t i = 0; i < N; ++i)
                state[i] = arr[i];
        }
        
        constexpr CharT operator[](std::size_t i) const { return state[i]; }
        constexpr CharT& operator[](std::size_t i) { return state[i]; }

        constexpr explicit operator CharT const*() const { return state; }
        constexpr CharT const* data() const { return state; }
        constexpr std::size_t size() const { return N; }
        constexpr CharT const* begin() const { return state; }
        constexpr CharT const* end() const { return begin() + size(); }

        constexpr fixed_string() = default;
        constexpr fixed_string(fixed_string const&) = default;
        constexpr fixed_string& operator=(fixed_string const&) = default;

        template<std::size_t M>
        friend constexpr fixed_string<CharT, N + M> operator+(fixed_string lhs, fixed_string<CharT, M> rhs)
        {
            fixed_string<CharT, N + M> retval;
            for (std::size_t i = 0; i < N; ++i)
                retval[i] = lhs[i];
            for (std::size_t i = 0; i < M; ++i)
                retval[N + i] = rhs[i];
            return retval;
        }

        friend constexpr bool operator==(fixed_string lhs, fixed_string rhs)
        {
            for (std::size_t i = 0; i < N; ++i)
                if (lhs[i] != rhs[i]) return false;
            return true;
        }

        friend constexpr bool operator!=(fixed_string lhs, fixed_string rhs)
        {
            for (std::size_t i = 0; i < N; ++i)
                if (lhs[i] != rhs[i]) return true;
            return false;
        }
        
        template<std::size_t M, std::enable_if_t<M != N, bool> = true>
        friend constexpr bool operator!=(fixed_string, fixed_string<CharT, M>)
        {
            return true;
        }

        template<std::size_t M, std::enable_if_t<M != N, bool> = true>
        friend constexpr bool operator==(fixed_string, fixed_string<CharT, M>)
        {
            return false;
        }

        operator std::basic_string<CharT>() const
        {
            return std::basic_string<CharT>(data(), size());
        }

        operator std::basic_string_view<const CharT>() const
        {
            return std::basic_string_view<CharT>(data(), size());
        }

        operator std::basic_string_view<CharT>()
        {
            return std::basic_string_view<CharT>(data(), size());
        }

        static constexpr fixed_string from_ascii(const char(&arr)[N + 1])
        {
            fixed_string s;
            
            for (std::size_t i = 0; i < N; ++i)
            {
                s.state[i] = static_cast<CharT>(arr[i]);
            }

            return s;
        }

    private:

        CharT state[N + 1] = { 0 };
    };

    template<typename CharT, std::size_t N>
    fixed_string(CharT const(&)[N])->fixed_string<CharT, N - 1>;
}
