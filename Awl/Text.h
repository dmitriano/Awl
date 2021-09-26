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
    class text
    {
    public:

        constexpr text(CharT const(&arr)[N + 1])
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

        constexpr text() = default;
        constexpr text(text const&) = default;
        constexpr text& operator=(text const&) = default;

        template<std::size_t M>
        friend constexpr text<CharT, N + M> operator+(text lhs, text<CharT, M> rhs)
        {
            text<CharT, N + M> retval;
            for (std::size_t i = 0; i < N; ++i)
                retval[i] = lhs[i];
            for (std::size_t i = 0; i < M; ++i)
                retval[N + i] = rhs[i];
            return retval;
        }

        friend constexpr bool operator==(text lhs, text rhs)
        {
            for (std::size_t i = 0; i < N; ++i)
                if (lhs[i] != rhs[i]) return false;
            return true;
        }

        friend constexpr bool operator!=(text lhs, text rhs)
        {
            for (std::size_t i = 0; i < N; ++i)
                if (lhs[i] != rhs[i]) return true;
            return false;
        }
        
        template<std::size_t M, std::enable_if_t<M != N, bool> = true>
        friend constexpr bool operator!=(text, text<CharT, M>)
        {
            return true;
        }

        template<std::size_t M, std::enable_if_t<M != N, bool> = true>
        friend constexpr bool operator==(text, text<CharT, M>)
        {
            return false;
        }

        operator std::basic_string<CharT>()
        {
            return std::basic_string<CharT>(data(), size());
        }

        operator std::basic_string_view<CharT>()
        {
            return std::basic_string_view<CharT>(data(), size());
        }

    private:

        CharT state[N + 1] = { 0 };
    };

    template<typename CharT, std::size_t N>
    text(CharT const(&)[N])->text<CharT, N - 1>;
}
